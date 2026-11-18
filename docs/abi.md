# ABI 契约（syscall 表）

版本：1（`FD_API_VERSION`）

每个模组必须先调用 `fd_core_version()` 并核对 `FD_API_VERSION`，不匹配要立即失败。

## 通用

| 符号 | 签名 | 说明 |
|---|---|---|
| `fd_core_version` | `int fd_core_version(void)` | 返回 `FD_API_VERSION` |
| `FdVec3` | `struct { float x, y, z; }` | 三维向量 |
| `FdStatus` | 枚举 | `FD_OK=0` `FD_ERR_VERSION=1` `FD_ERR_ARGUMENT=2` `FD_ERR_STATE=3` |

## 街机滑板车

| 符号 | 签名 | 说明 |
|---|---|---|
| `fd_vehicle_preset` | `FdStatus fd_vehicle_preset(int32_t preset, FdVehicleTuning *out)` | 填默认调校；preset: MILD/WILD/ABSURD |
| `fd_vehicle_step` | `FdStatus fd_vehicle_step(FdVehicle *v, const FdVehicleInput *input, int32_t preset, int32_t riders, float dt)` | dt 必须是 `FD_VEH_FIXED_DT`，否则返回 `FD_ERR_ARGUMENT` |
| `fd_vehicle_collide` | `int32_t fd_vehicle_collide(FdVehicle *v, float nx, float nz, int32_t kind, float *out_impact)` | kind: 0 墙 / 1 道具 / 2 对手；返回 1 表示撞了 |

## 手动挡车

| 符号 | 签名 | 说明 |
|---|---|---|
| `fd_car_default_tuning` | `void fd_car_default_tuning(FdCarTuning *out)` | 填默认调校 |
| `fd_car_apply_tuning` | `FdStatus fd_car_apply_tuning(FdCar *c, const FdCarTuning *tuning)` | 热刷新：只改配置，保留运行时状态 |
| `fd_car_step` | `FdStatus fd_car_step(FdCar *c, const FdCarInput *input, float dt)` | dt 必须是 `FD_VEH_FIXED_DT`；故障边沿触发在 `c->events` |
| `fd_car_restart` | `FdStatus fd_car_restart(FdCar *c)` | 清除引擎故障：原地重启，保留位置与朝向，挡位归空挡、速度归零 |
| `fd_car_collide` | `int32_t fd_car_collide(FdCar *c, float nx, float nz, int32_t kind, float *out_impact)` | kind: 0 墙 / 1 道具 / 2 对手 |

## 故障事件（FdCar.events 位域）

| 位 | 含义 |
|---|---|
| `FD_CAR_EVENT_ENGINE_FAILED` | 超转（转速超过红线）→ 引擎故障，本步触发 |
| `FD_CAR_EVENT_GEAR_GRIND` | 没踩离合换挡 → 打齿，本步触发 |

## 挡位（FdGear）

`FD_GEAR_NEUTRAL=0`、`FD_GEAR_1..5`、`FD_GEAR_REVERSE=-1`。
