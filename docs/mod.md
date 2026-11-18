# 写一个载具模组（mod）

版本：1（`FD_MOD_API_VERSION`）

模组是一个原生动态库（Linux `.so` / Windows `.dll` / macOS `.dylib`），由宿主
open-fdv 在启动时扫描并加载。模组只负责**声明数据**：它要提供哪些载具、每种载具用
哪套规则、初始调校是什么。物理在核心里跑，不在模组里跑——这就是把 open-fdv 当内核
的意思：模组是"程序"，内核负责执行。

## 契约

一个模组必须导出下面四个符号（`fd_mod.h`）：

| 符号 | 签名 | 说明 |
|---|---|---|
| `fd_mod_info` | `const FdModInfo *fd_mod_info(void)` | 静态描述，最先被调用。`abi_version` 必须等于 `FD_MOD_API_VERSION`，否则宿主直接拒绝，不读其他字段 |
| `fd_mod_init` | `int32_t fd_mod_init(const FdModHost *host)` | 版本核对通过后调用一次。返回 `FD_OK` 才会被登记 |
| `fd_mod_vehicle_count` | `int32_t fd_mod_vehicle_count(void)` | 本模组提供的载具数量 |
| `fd_mod_vehicle` | `const FdModVehicle *fd_mod_vehicle(int32_t index)` | 第 `index` 个载具；越界返回 `NULL`。指针在进程结束前必须一直有效 |

可选导出 `fd_mod_shutdown`，宿主卸载前调用。当前宿主在整个进程生命周期内保持模组
加载，所以它只是留给以后用。

## 载具定义

```c
typedef struct {
        const char *id;           /* 稳定主键，会写进存档 */
        const char *name;
        const char *description;
        int32_t kind;             /* FD_MOD_VEHICLE_SCOOTER / FD_MOD_VEHICLE_CAR */
        int32_t preset;           /* 滑板车：FdHandlingPreset */
        FdCarTuning car;          /* 手动挡车：初始调校 */
        float body_scale;         /* 外观提示，1.0 = 原尺寸 */
        float body_colour[3];     /* 外观提示，线性 RGB；全 0 = 宿主默认 */
} FdModVehicle;
```

`kind` 选择内核已有的规则集，模组不能自带物理。`car` 里的调校就是
`fd_api.h` 的 `fd_car_apply_tuning()` 接受的那一组字段（扭矩、质量、风阻、转向速率、
齿比等）。

**不要给 `FdCarTuning` 清零后直接填。** 清零 = 扭矩 0、红线 0，车一启动就故障。
先通过宿主服务拿默认值：

```c
FdCarTuning tuning;
host->car_default_tuning(&tuning);
tuning.mass = 1700.0f;
```

## 宿主服务（FdModHost）

| 字段 | 说明 |
|---|---|
| `abi_version` | 宿主实现的 ABI 版本，便于模组反向核对 |
| `log(level, message)` | 一条诊断输出；`message` 只在调用期间有效。级别：`FD_MOD_LOG_INFO/WARN/ERROR` |
| `car_default_tuning(out)` | 内核当前的汽车默认调校，避免把会漂移的数字写死在模组里 |

目前没有回调式 API：模组不参与每帧逻辑，只声明。以后要加"每帧自定义行为"会走新的
ABI 版本，而不是悄悄改这个结构体。

## 生命周期

1. 宿主扫描模组目录，逐个 `dlopen` / `LoadLibrary`。
2. 取 `fd_mod_info`，先比对 `abi_version`（不匹配就关闭库并记录原因）。
3. 调 `fd_mod_init`，把宿主服务表递过去。
4. 调 `fd_mod_vehicle_count` / `fd_mod_vehicle`，把每个载具**复制**成宿主自己的数据。
5. 宿主把载具列表交给游戏；游戏负责价格、解锁、UI 等业务。

扫描失败（目录不存在、缺少符号、版本不符、初始化失败）都会作为一条带 `ok = false`
和 `error` 的记录返回，不会中断其他模组——一个坏模组不该让所有模组失效。

## 打包与安装

```sh
# Linux
cc -shared -fPIC -I../include example_mod.c -o example_mod.so
# macOS
cc -shared -fPIC -I../include example_mod.c -o example_mod.dylib
# Windows (MSVC)
cl /LD /I..\include example_mod.c /Fe:example_mod.dll
```

把产物放进游戏的模组目录（`user://mods`、可执行文件旁的 `mods/`，或开发时的
`res://mods/`）。目录里除了动态库不需要别的文件：元数据全在 `fd_mod_info` 里。

## 信任边界

模组是原生代码，加载后拥有和游戏相同的权限。ABI 是**稳定性**契约，不是沙箱。只加载
你信任的模组。

## 版本

`FD_MOD_API_VERSION` 只在结构体布局或调用顺序变化时递增；新增一个可选导出符号不算
破坏性变更。宿主拒绝版本不匹配的模组，而不是尽力兼容——读错结构体比拒绝加载更糟。

## 示例

- `examples/example_mod.c`：两个载具（一辆面包车、一辆双人滑板车），完整实现上述四个
  导出。
- `examples/minimal_mod.c`：直接针对 `fd_api.h` 调内核 syscall 的最小程序，与加载器
  无关，用来理解 ABI 本身。
