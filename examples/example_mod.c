/*
 * example_mod.c — a complete mod in one file.
 *
 * It adds two vehicles: a heavy van and a two-rider scooter. Nothing here is
 * Godot-specific; the host reads these definitions, copies them, and drives
 * them with its own rules. That is the whole contract.
 *
 * Build (Linux):
 *   cc -shared -fPIC -I../include example_mod.c -o example_mod.so
 *
 * Build (macOS):
 *   cc -shared -fPIC -I../include example_mod.c -o example_mod.dylib
 *
 * Build (Windows, MSVC):
 *   cl /LD /I..\include example_mod.c /Fe:example_mod.dll
 *
 * Copy the result into the game's mods directory. The host scans that
 * directory on startup; nothing else is needed.
 *
 * Apache-2.0.
 */
#include <stddef.h>
#include <string.h>

#include <fd_mod.h>

static const FdModHost *host_services = NULL;
static FdCarTuning van_tuning;
static FdModVehicle vehicles[2];

static const FdModInfo INFO = {
        FD_MOD_API_VERSION,
        "com.example.example-pack",
        "示例载具包",
        "1.0.0",
        "open-fdv-dev",
        "一辆载重面包车和一辆双人滑板车，用来演示 mod ABI。",
};

FD_MOD_EXPORT const FdModInfo *fd_mod_info(void)
{
        return &INFO;
}

static void note(int32_t level, const char *text)
{
        if (host_services != NULL && host_services->log != NULL) {
                host_services->log(level, text);
        }
}

FD_MOD_EXPORT int32_t fd_mod_init(const FdModHost *host)
{
        if (host == NULL || host->abi_version != FD_MOD_API_VERSION) {
                return FD_ERR_VERSION;
        }
        host_services = host;

        /* Start from the kernel's defaults rather than a zeroed struct. A zeroed
         * FdCarTuning is a car with no torque, a zero redline, and therefore an
         * engine that fails the instant it turns over. */
        if (host->car_default_tuning == NULL) {
                return FD_ERR_STATE;
        }
        memset(&van_tuning, 0, sizeof(van_tuning));
        host->car_default_tuning(&van_tuning);

        /* A van, not a sports car: more torque, much more mass, and slower
         * steering, so it feels like the delivery it is carrying. */
        van_tuning.max_torque = 240.0f;
        van_tuning.mass = 1700.0f;
        van_tuning.drag = 0.055f;
        van_tuning.steer_rate = 1.3f;
        van_tuning.final_drive_ratio = 4.3f;

        memset(vehicles, 0, sizeof(vehicles));

        vehicles[0].id = "heavy_van";
        vehicles[0].name = "载重面包车";
        vehicles[0].description = "慢、重、稳，撞一下汤汁不一定洒。";
        vehicles[0].kind = FD_MOD_VEHICLE_CAR;
        vehicles[0].car = van_tuning;
        vehicles[0].body_scale = 1.12f;
        vehicles[0].body_colour[0] = 0.28f;
        vehicles[0].body_colour[1] = 0.42f;
        vehicles[0].body_colour[2] = 0.55f;

        vehicles[1].id = "courier_scooter";
        vehicles[1].name = "双人快递车";
        vehicles[1].description = "两个人一辆车，转弯时谁也别松手。";
        vehicles[1].kind = FD_MOD_VEHICLE_SCOOTER;
        vehicles[1].preset = FD_HANDLING_WILD;
        vehicles[1].body_scale = 1.0f;
        vehicles[1].body_colour[0] = 0.90f;
        vehicles[1].body_colour[1] = 0.55f;
        vehicles[1].body_colour[2] = 0.15f;

        note(FD_MOD_LOG_INFO, "example pack ready");
        return FD_OK;
}

FD_MOD_EXPORT int32_t fd_mod_vehicle_count(void)
{
        return 2;
}

FD_MOD_EXPORT const FdModVehicle *fd_mod_vehicle(int32_t index)
{
        if (index < 0 || index >= 2) {
                return NULL;
        }
        return &vehicles[index];
}

FD_MOD_EXPORT void fd_mod_shutdown(void)
{
        host_services = NULL;
}
