# open-fdv-dev

开放 API + SDK，用于给 open-fdv 载具模块写第三方模组。

## 这是什么

open-fdv-dev 是公开的 Apache-2.0 SDK，提供一张稳定的 C ABI（"syscall 表"），
模组针对这张表开发，不关心上层是 Unity 还是 Godot。

## 架构（Open-Core）

- **C ABI（本 SDK）** — 稳定、版本化、引擎无关的接口，模组调用的 "syscall"。公开。
- **open-fdv** — 绑定 C ABI 的 Godot GDExtension。公开。
- **Fatal-Delivery** — 闭源游戏（业务层）。
- **GDScript / Lua** — 配置层（脚本，热加载）。

C ABI 是"内核"，模组是"程序"，通过稳定、编号的接口调用它——像系统调用一样。

## 目录

- include/fd_api.h — C ABI 声明（syscall 表）
- include/fd_mod.h — 模组契约：模组导出什么、宿主提供什么
- docs/abi.md — ABI 契约，一条 syscall 一条说明
- docs/mod.md — 怎么打包一个载具模组、加载顺序、信任边界
- examples/minimal_mod.c — 直接调用 syscall 的最小程序
- examples/example_mod.c — 一个完整的载具模组（两个载具）
- LICENSE — Apache-2.0

## 写一个模组

1. 参照 `docs/mod.md`，实现 `fd_mod_info` / `fd_mod_init` /
   `fd_mod_vehicle_count` / `fd_mod_vehicle` 四个导出。
2. 用 `fd_api.h` 里的 `FdCarTuning` 和 `FdHandlingPreset` 描述载具；不要清零结构体，
   用宿主给的 `car_default_tuning()`。
3. 编译成动态库，放进游戏的模组目录。open-fdv 扫描、校验 ABI、加载，游戏负责把载具
   列进选择界面。
4. 直接调 syscall（而不是只声明载具）时，再 `include fd_api.h`，先调
   `fd_core_version()` 核对 `FD_API_VERSION`，然后调用其他 syscall。
