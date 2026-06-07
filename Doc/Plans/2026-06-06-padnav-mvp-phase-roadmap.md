# PadNav MVP 阶段路线图

## 1. 文档目的

这份文档用于你自己实现 PadNav MVP（最小可行产品）时按阶段推进。

当前路线图采用 Vertical Slice（纵向切片）开发顺序：先让程序能被观察和调试，再逐步补齐 Core、WindowsPlatform、Gui 和最终后台运行模型。这个顺序比纯自底向上更适合人工开发。

详细任务级实现参考：

- `Doc/Requirements/PadNav-MVP-Requirements.md`
- `Doc/Plans/2026-05-30-padnav-mvp-implementation-plan.md`

## 2. 模块位置

| 模块 | Target（目标） | 项目位置 |
| --- | --- | --- |
| Core：仅使用标准 C++20 | `Pn::PadNavCore` | `Src/Core/Interface/PadNav` |
| Platform：仅 Win32 | `Pn::WindowsPlatform` | `Src/Core/Interface/WindowsPlatform` |
| Application：标准 C++20 编排层 | `Pn::Application` | `Src/Core/Interface/Application` |
| Gui：Qt 边界 | `Pn::PadNavGui` | `Src/Gui/Scene/PadNav` |
| App | `Pn::PadNav` / `padnav` | `Src/App` |
| tests | `padnav_tests` | `Tests/PnTests` |

说明：本项目只适配 Windows 平台，因此 `WindowsPlatform` 直接放在 `Src/Core/Interface/WindowsPlatform`，不保留 `Src/Platform` 聚合模块。

## 3. 管理类

综合管理类名称：

```text
PadNavController
```

位置：

```text
Src/Core/Interface/Application/padnavcontroller.h
Src/Core/Interface/Application/padnavcontroller.cpp
```

`PadNavController` 是 main.cpp 创建并持有的 single owner（单一拥有者），不是硬 Singleton（单例）。UI 通过引用使用它，不通过 `static instance()` 全局访问。

## 4. 总体阶段

| 阶段 | 名称 | 结果 |
| --- | --- | --- |
| 阶段 0 | 工程骨架与验证基线 | CMake、target、alias 和 `padnav_tests` 跑通 |
| 阶段 1 | `PadNavController` 调试壳 | `main.cpp + QTimer + pollOnce()` 可驱动一帧 |
| 阶段 2 | XInput 可观察输入 | 能读取真实北通手柄并显示状态 |
| 阶段 3 | SendInput 可观察输出 | 能单独验证鼠标、滚轮和快捷键注入 |
| 阶段 4 | Core Domain 与 MappingEngine | 映射规则接入真实输入输出闭环 |
| 阶段 5 | Input Monitor 与 SettingsWindow | UI 可以观察状态并调整启停和参数 |
| 阶段 6 | JSON 配置持久化 | 设置可以保存和恢复 |
| 阶段 7 | 后台线程收敛与 Crash Dump | `QTimer` 调试轮询收敛为 `std::jthread`，并生成 dmp |
| 阶段 8 | 手工验收与体验微调 | Chrome 连续浏览体验通过验收 |

## 5. 阶段 0：工程骨架与验证基线

### 5.1 目标

让项目结构和 target dependency（目标依赖）先成立。

### 5.2 功能点

- `Pn::PadNavCore` 存在。
- `Pn::WindowsPlatform` 存在。
- `Pn::Application` 存在。
- `Pn::PadNavGui` 存在。
- `padnav_tests` 存在。
- 各模块 `target_link_libraries` 使用别名目标。

### 5.3 依赖方向

```text
Gui -> Application -> Core
Application -> WindowsPlatform -> Core
Tests/PnTests -> Core/Application/WindowsPlatform/Gui
```

### 5.4 验收标准

```powershell
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target padnav_tests
ctest --test-dir cmake-build-debug -R padnav_tests --output-on-failure
```

如果 `kernel32.lib` 找不到，优先修 MSVC / Windows SDK 环境。

## 6. 阶段 1：PadNavController 调试壳

### 6.1 目标

先建立一个可由 `QTimer` 驱动的运行入口，方便人工逐步观察和调试。

### 6.2 功能点

- 创建 `PadNavController`。
- 提供 `pollOnce()`。
- 提供 `setMappingEnabled()` / `mappingEnabled()`。
- 提供 `monitorSnapshot()`。
- `start()` / `stop()` 可先保留空实现，后续阶段再接 `std::jthread`。
- `main.cpp` 创建唯一 `PadNavController` 实例。
- `main.cpp` 使用 `QTimer` 每 `8 ms` 调用一次 `pollOnce()`。

### 6.3 Ownership（所有权）

```text
main.cpp owns PadNavController
PadNavController owns XInputController / SendInputEmitter / MappingEngine
UI holds PadNavController&
```

### 6.4 Thread Affinity（线程亲和性）

此阶段不使用后台线程。

```text
GUI Thread
    -> QTimer
    -> PadNavController::pollOnce()
```

### 6.5 Trade-off（权衡）

优点是每一步都能观察结果，适合人工调试。代价是早期 `main.cpp` 会有临时代码，后续必须收敛到后台线程。

## 7. 阶段 2：XInput 可观察输入

### 7.1 目标

确认北通手柄能被真实读取。

### 7.2 功能点

- 实现 `XInputController::poll()`。
- 读取 controller index `0`。
- 连接失败时返回 `connected = false`。
- 标准化左右摇杆为 `-1.0F` 到 `1.0F`。
- 将 XInput buttons 映射为 `ControllerButton` bit mask。
- `PadNavController::pollOnce()` 更新 `MonitorSnapshot`。

### 7.3 验收标准

- 手柄连接状态可观察。
- A/B/X/Y、LB/RB、D-Pad、View 可观察。
- 左右摇杆数值变化可观察。
- 断开手柄不崩溃。

## 8. 阶段 3：SendInput 可观察输出

### 8.1 目标

先单独验证 Windows 输入注入能力。

### 8.2 功能点

- 实现 `SendInputEmitter::emit()`。
- 支持 `MouseMove`。
- 支持 `Scroll`。
- 支持左键和右键 press/release。
- 支持 `KeyChord`。
- 组合键顺序固定为 modifier down、main down、main up、modifier up。

### 8.3 验收标准

- Chrome 中可以通过手柄触发鼠标移动。
- 可以触发左键、右键。
- 可以触发滚轮。
- 可以触发至少一个快捷键，例如 `Alt+Left` 或 `Ctrl+R`。

## 9. 阶段 4：Core Domain 与 MappingEngine

### 9.1 目标

把固定 Chrome Profile（配置模板）和映射规则接入真实输入输出闭环。

### 9.2 功能点

- 实现 `ControllerSnapshot`、`Action`、`ChromeProfile`。
- 实现默认参数和范围校验。
- 实现 `MappingEngine::map()`。
- 支持 Base Layer、Navigation Layer、Tab Layer。
- 支持 `LB + RB` 歧义层。
- 支持 Rising Edge（上升沿）。
- 支持 Dead Zone（死区）。
- 支持单项动作启停。
- `PadNavController::pollOnce()` 串联：

```text
XInputController::poll()
    -> MappingEngine::map()
    -> SendInputEmitter::emit()
    -> update MonitorSnapshot
```

### 9.3 验收标准

- `A` 触发左键。
- `B` 触发右键。
- `X` / `Y` 触发 Chrome 前进后退。
- `LB` / `RB` 修饰层工作。
- Dead Zone 内摇杆不移动。
- 长按快捷键不会重复触发。

## 10. 阶段 5：Input Monitor 与 SettingsWindow

### 10.1 目标

补上必要 UI，用于观察和调参。

### 10.2 功能点

- `InputMonitorWindow` 显示连接状态、摇杆、按键、Layer、启停状态。
- `SettingsWindow` 调整 Dead Zone、鼠标速度、滚动速度。
- `SettingsWindow` 支持动作启停。
- 托盘菜单提供启用 / 暂停、打开设置、退出。

### 10.3 Thread Affinity（线程亲和性）

所有 QWidget 位于 GUI Thread。UI 使用 `QTimer` 读取 `PadNavController::monitorSnapshot()`。

## 11. 阶段 6：JSON 配置持久化

### 11.1 目标

让设置在退出后保留。

### 11.2 功能点

- 使用 `QJsonObject`、`QJsonDocument`、`QFile` 或 `QSaveFile`。
- 保存到 `%APPDATA%/PadNav/config.json`。
- 保存 Dead Zone、鼠标速度、滚动速度、动作启停。
- 文件缺失或损坏时静默恢复默认配置。

### 11.3 验收标准

- 点击“应用”后写入 JSON。
- 重启后配置恢复。
- 点击“取消”后未应用修改丢弃。

## 12. 阶段 7：后台线程收敛与 Crash Dump

### 12.1 目标

将调试阶段的 `QTimer` 轮询收敛为最终后台运行模型，并补 Crash Dump。

### 12.2 功能点

- `PadNavController::start()` 创建 `std::jthread`。
- `PadNavController::stop()` 请求停止并等待线程退出。
- 线程内每约 `8 ms` 调用 `pollOnce()` 或等价内部函数。
- UI 不再驱动主轮询，只读取 snapshot。
- 安装 `SetUnhandledExceptionFilter`。
- 使用 `MiniDumpWriteDump` 写入 `MiniDumpNormal`。
- dmp 保存到 `%LOCALAPPDATA%/PadNav/CrashDumps/`。
- 最多保留最近 `3` 个 dmp。

### 12.3 验收标准

- 关闭设置窗口后后台映射继续运行。
- 退出时线程能正常停止。
- 未处理异常生成 dmp。

## 13. 阶段 8：手工验收与体验微调

### 13.1 目标

验证 PadNav 是否真正适合 Chrome 浏览。

### 13.2 验收标准

- Chrome 连续浏览至少 `15` 分钟。
- 鼠标移动无明显卡顿、漂移或误触。
- 滚动方向正确且连续。
- Base / Navigation / Tab Layer 均可用。
- 暂停后不注入键鼠事件。
- 手柄断开后应用不崩溃。
- 手柄重连后自动恢复。

## 14. 不建议提前做的内容

- 任意改键。
- 宏录制。
- On-Screen Keyboard（屏幕键盘）。
- Chrome 前台进程自动检测。
- 开机启动。
- 多手柄。
- DirectInput。
- Raw Input。
- SDL3。
- 虚拟手柄驱动。
- UI 主题美化。

