/*
 * minimal_mod.c — the smallest possible vehicle mod.
 *
 * Links against the game's compiled core, checks the ABI version, fetches a
 * car tuning, applies it, and steps one fixed tick.
 */
#include <fd_api.h>
#include <stdio.h>

int main(void)
{
        FdCar car = {0};
        FdCarTuning tuning;
        FdCarInput input = {0};

        if (fd_core_version() != FD_API_VERSION) {
                fprintf(stderr, "ABI mismatch: expected %d\n", FD_API_VERSION);
                return 1;
        }

        fd_car_default_tuning(&tuning);
        input.throttle = 1.0f;
        input.requested_gear = FD_GEAR_1;

        fd_car_step(&car, &input, FD_VEH_FIXED_DT);

        printf("rpm=%.1f gear=%d speed=%.2f\n", car.rpm, car.gear, car.speed);
        return 0;
}
