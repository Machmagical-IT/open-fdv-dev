/*
 * fd_api.h — the public, stable, versioned vehicle ABI.
 *
 * This is the "syscall table" third-party mods develop against. It is a public
 * declaration of the interface only, not the implementation: the game satisfies
 * this contract, and a mod links against the compiled core.
 *
 * ABI rules (the kernel-syscall contract):
 *
 *   - Versioned: call fd_core_version() first and compare to FD_API_VERSION; a
 *     mismatch must fail loudly rather than read garbage.
 *   - POD in / POD out, FdStatus return codes, no cross-boundary exceptions.
 *   - Deterministic and fixed-step: the step functions accept only
 *     FD_VEH_FIXED_DT; any other dt returns FD_ERR_ARGUMENT.
 *
 * Apache-2.0.
 */
#ifndef FD_API_H
#define FD_API_H

#include <stddef.h>
#include <stdint.h>

#define FD_API_VERSION 4

/* The core is compiled as C and exports unmangled names. A mod (or the
 * open-fdv binding) that includes this header from C++ must see the same
 * linkage, or every call compiles to a mangled undefined symbol and the
 * library fails to load at runtime. */
#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------ */
/* Common                                                              */
/* ------------------------------------------------------------------ */

typedef struct {
        float x, y, z;
} FdVec3;

typedef enum {
        FD_OK = 0,
        FD_ERR_VERSION = 1,
        FD_ERR_ARGUMENT = 2,
        FD_ERR_STATE = 3,
} FdStatus;

/* ------------------------------------------------------------------ */
/* Arcade scooter                                                      */
/* ------------------------------------------------------------------ */

typedef enum {
        FD_HANDLING_MILD = 0,
        FD_HANDLING_WILD = 1,
        FD_HANDLING_ABSURD = 2,
} FdHandlingPreset;

typedef enum {
        FD_VEH_UPRIGHT = 0,
        FD_VEH_SKIDDING = 1,
        FD_VEH_FLIPPING = 2,
        FD_VEH_WRECKED = 3,
} FdVehicleState;

typedef struct {
        float x, z;
        float heading;
        float speed;
        float lateral_speed;
        float lean;
        float steer_angle;
        int32_t state;
        float state_timer;
} FdVehicle;

typedef struct {
        float throttle;
        float steer;
        int32_t steer_assist;
} FdVehicleInput;

typedef struct {
        int32_t preset;
        float max_speed;
        float acceleration;
        float steer_rate;
        float grip;
        float slide_force;
        float lean_per_lateral;
        float max_lean;
        float flip_lean_threshold;
        float flip_duration;
        float skid_lateral_threshold;
} FdVehicleTuning;

#define FD_VEH_FIXED_DT 0.02f

/* ------------------------------------------------------------------ */
/* Manual-transmission car                                             */
/* ------------------------------------------------------------------ */

typedef enum {
        FD_GEAR_NEUTRAL = 0,
        FD_GEAR_1 = 1,
        FD_GEAR_2 = 2,
        FD_GEAR_3 = 3,
        FD_GEAR_4 = 4,
        FD_GEAR_5 = 5,
        FD_GEAR_REVERSE = -1,
} FdGear;

typedef enum {
        FD_CAR_EVENT_NONE = 0,
        FD_CAR_EVENT_ENGINE_FAILED = 1 << 0,
        FD_CAR_EVENT_GEAR_GRIND = 1 << 1,
} FdCarEvent;

typedef struct {
        float max_torque;
        float idle_rpm;
        float redline_rpm;
        float gear_ratios[6];
        int32_t gear_count;
        float final_drive_ratio;
        float wheel_radius;
        float mass;
        float drag;
        float steer_rate;
        float grip;             /* lateral velocity decay, 1/s */
        float handbrake_grip;   /* same, while the handbrake is held */
        float max_steer_angle;  /* wheel lock, normalized */
        float steer_speed;      /* how fast the wheel follows the input, 1/s */
} FdCarTuning;

typedef struct {
        float x, z;
        float heading;
        float steer_angle;      /* the wheel: state, clamped at max_steer_angle */
        float speed;            /* m/s along heading */
        float lateral_speed;    /* m/s sideways, positive left: the drift */
        float rpm;
        int32_t gear;
        float clutch;
        int32_t engine_failed;
        int32_t drivetrain_open; /* 1 after a grind until the clutch is pressed */
        int32_t events;
        FdCarTuning tuning;
} FdCar;

typedef struct {
        float throttle;
        float steer;
        float clutch;
        int32_t requested_gear;
        int32_t handbrake;      /* 0/1: less grip, more yaw, scrubs speed */
} FdCarInput;

/* ------------------------------------------------------------------ */
/* Syscalls                                                            */
/* ------------------------------------------------------------------ */

/* Returns FD_API_VERSION. Check this before anything else. */
int fd_core_version(void);

/* Arcade scooter. */
FdStatus fd_vehicle_preset(int32_t preset, FdVehicleTuning *out);
FdStatus fd_vehicle_step(FdVehicle *v, const FdVehicleInput *input,
                         int32_t preset, int32_t riders, float dt);
int32_t fd_vehicle_collide(FdVehicle *v, float normal_x, float normal_z,
                           int32_t kind, float *out_impact_speed);

/* Manual-transmission car. */
void fd_car_default_tuning(FdCarTuning *out);
FdStatus fd_car_apply_tuning(FdCar *c, const FdCarTuning *tuning);
FdStatus fd_car_step(FdCar *c, const FdCarInput *input, float dt);
/* Clear a failed engine and restart in place: keeps position and heading,
 * drops the gear to neutral and the speed to zero. The fault is sticky, so
 * this is the only way back. */
FdStatus fd_car_restart(FdCar *c);
int32_t fd_car_collide(FdCar *c, float normal_x, float normal_z,
                       int32_t kind, float *out_impact_speed);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* FD_API_H */
