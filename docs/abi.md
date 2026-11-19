# ABI 契约（syscall 表）

版本：5（`FD_API_VERSION`）

v5 变更：`fd_box_throw_dir` 新增投掷者的水平速度 `carrier_vx` / `carrier_vz`
（开车时是车速，步行时是胶囊速度），签名变化，旧模组必须重编。v4 增加
`FdCar.drivetrain_open`；v3 增加 `steer_angle`（轮角状态）与
`max_steer_angle` / `steer_speed`；v2 已增加 `lateral_speed`、`handbrake`、
`grip` / `handbrake_grip`。这些是结构体布局或函数签名变更，旧模组必须重编。

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

`FdCarInput.handbrake`（0/1）降低横向抓地、提高偏航并消耗前进速度；
`FdCar.lateral_speed` 是车头方向与速度方向的差，也就是漂移量。

## 配送箱

| 符号 | 签名 | 说明 |
|---|---|---|
| `fd_box_throw_dir` | `int32_t fd_box_throw_dir(float fx, float fy, float fz, float tx, float ty, float tz, float carrier_vx, float carrier_vz, uint32_t seed, float gravity, FdVec3 *out)` | 初速度 = 方向解 + 投掷者水平速度，水平分量夹在 `FD_BOX_THROW_MAX_SPEED`（基础 14 m/s 的 1.6 倍）；垂直分量不缩放。`carrier_v* = 0` 与 v4 弧线逐位一致 |

`seed` 让摆动在两端一致，`carrier_vx` / `carrier_vz` 让箱子继承投掷者的动量：
向前投更远、反向投更近，120 km/h 的车也不会把箱子当炮弹打出去。

## 故障事件（FdCar.events 位域）

| 位 | 含义 |
|---|---|
| `FD_CAR_EVENT_ENGINE_FAILED` | 超转（转速超过红线）→ 引擎故障，本步触发 |
| `FD_CAR_EVENT_GEAR_GRIND` | 没踩离合换挡 → 打齿，本步触发 |

## 挡位（FdGear）

`FD_GEAR_NEUTRAL=0`、`FD_GEAR_1..5`、`FD_GEAR_REVERSE=-1`。
