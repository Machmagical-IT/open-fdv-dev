/*
 * fd_mod.h — how a native vehicle mod is discovered and read.
 *
 * A mod is a shared library (.so / .dll / .dylib) exporting the symbols at the
 * bottom of this file. The host (open-fdv) opens it, checks `abi_version`,
 * calls the initialiser, and asks for the vehicle definitions the mod wants to
 * add to the game. Nothing here is engine-specific: a mod declares data, and
 * the kernel drives it with the syscalls in fd_api.h. That is deliberate --
 * a mod must not need to know whether it is running under Godot or Unity.
 *
 * ABI rules:
 *
 *   - Versioned: the host refuses a mod whose `abi_version` differs from
 *     FD_MOD_API_VERSION. It reads no other field first, because an older
 *     struct layout read as a newer one is garbage.
 *   - POD in / POD out. No callbacks from the simulation thread, no C++ types,
 *     no exceptions, no allocation across the boundary.
 *   - The host owns the storage. Strings stay valid for as long as the library
 *     is loaded; the host copies what it needs at scan time.
 *   - Native code is not sandboxed. A mod runs with the game's privileges; the
 *     ABI is a contract, not a security boundary.
 *
 * Apache-2.0.
 */
#ifndef FD_MOD_H
#define FD_MOD_H

#include <stddef.h>
#include <stdint.h>

#include <fd_api.h>

#define FD_MOD_API_VERSION 2

/* Log levels passed to FdModHost::log. */
#define FD_MOD_LOG_INFO 0
#define FD_MOD_LOG_WARN 1
#define FD_MOD_LOG_ERROR 2

/* Export the entry points from a mod. Visible on every platform: a shared
 * library built without default visibility would otherwise load cleanly and
 * then fail every dlsym, which is a confusing way to report "no symbols". */
#if defined(_WIN32)
#  define FD_MOD_EXPORT __declspec(dllexport)
#else
#  define FD_MOD_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* What the mod is. `id` is the stable key: the game persists it in the save
 * file, so renaming it loses the player's selection. Use reverse-DNS
 * ("com.example.van") or a short slug ("toolbox-van"). */
typedef struct {
        uint32_t abi_version;   /* must equal FD_MOD_API_VERSION */
        const char *id;
        const char *name;       /* shown in the vehicle picker */
        const char *version;   /* the mod's own version, free-form */
        const char *author;
        const char *description; /* one sentence, shown in the picker */
} FdModInfo;

/* Which rule set drives the vehicle. The kernel owns the physics; a mod
 * chooses one of the primitives it already knows how to step. */
typedef enum {
        FD_MOD_VEHICLE_SCOOTER = 0,
        FD_MOD_VEHICLE_CAR = 1,
} FdModVehicleKind;

/* One selectable vehicle. Fields not used by `kind` are ignored. */
typedef struct {
        const char *id;          /* unique within the mod */
        const char *name;
        const char *description;

        int32_t kind;            /* FdModVehicleKind */

        /* FD_MOD_VEHICLE_SCOOTER: start from this handling preset. */
        int32_t preset;          /* FdHandlingPreset */

        /* FD_MOD_VEHICLE_CAR: the tuning the car starts with. Fill it from
         * fd_mod_host->car_default_tuning() and override what you care about;
         * a zeroed struct is a car with no torque and a redline of zero. */
        FdCarTuning car;

        /* Presentation hints. The host may ignore them; they exist so a mod
         * can look different without shipping engine assets.
         *   body_scale   1.0 = the host's default body
         *   body_colour  linear RGB in 0..1; all zero = host default */
        float body_scale;
        float body_colour[3];
} FdModVehicle;

/* Functions the kernel lends the mod. `abi_version` is set by the host, so a
 * mod built against a newer host can check it before calling anything. */
typedef struct {
        uint32_t abi_version;

        /* Diagnostic line. `message` is NUL-terminated UTF-8 and is only valid
         * for the duration of the call. */
        void (*log)(int32_t level, const char *message);

        /* The kernel's default car tuning. Call this instead of hard-coding
         * numbers that will drift when the core retunes the car. */
        void (*car_default_tuning)(FdCarTuning *out);
} FdModHost;

/* ------------------------------------------------------------------ */
/* Entry points a mod must export                                      */
/* ------------------------------------------------------------------ */

/* Static description. Called before anything else, so it must not allocate or
 * depend on fd_mod_init(). May be called more than once. */
FD_MOD_EXPORT const FdModInfo *fd_mod_info(void);

/* Called once after the version check passes. Return FD_OK to be listed;
 * any other FdStatus makes the host skip the mod and log the failure. */
FD_MOD_EXPORT int32_t fd_mod_init(const FdModHost *host);

/* How many vehicles this mod adds. May be 0 (a library-only mod is legal but
 * useless today). */
FD_MOD_EXPORT int32_t fd_mod_vehicle_count(void);

/* Vehicle definition for index in [0, fd_mod_vehicle_count()). The pointer
 * must stay valid until the process exits. Return NULL for a bad index. */
FD_MOD_EXPORT const FdModVehicle *fd_mod_vehicle(int32_t index);

/* Optional: called before the library is unloaded. The current host keeps mods
 * loaded for the whole process, so this is only for completeness. */
FD_MOD_EXPORT void fd_mod_shutdown(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FD_MOD_H */
