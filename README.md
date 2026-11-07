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
- docs/abi.md — ABI 契约，一条 syscall 一条说明
- examples/ — 最小模组示例
- LICENSE — Apache-2.0

## 写一个模组

1. include fd_api.h。
2. 链接游戏编译出的 core（fd_core）。
3. 先调用 fd_core_version() 核对 FD_API_VERSION，再调用其他 syscall。
