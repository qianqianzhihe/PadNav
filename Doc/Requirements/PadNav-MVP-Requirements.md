# PadNav MVP 需求文档

## 设计第 1 部分：产品范围

### 1. 项目名称

项目名称：**PadNav**

名称含义：`Pad（手柄） + Navigation（导航）`。

### 2. 产品定位

PadNav 是面向 Windows 11 的轻量级手柄浏览器导航应用。

首个版本围绕 Chrome 浏览器优化，目标是通过游戏手柄流畅完成页面浏览、标签页管理和常用快捷键操作。PadNav 首版不是通用游戏改键器。

### 3. 已确认运行环境

#### 3.1 操作系统

- Windows 11

#### 3.2 手柄设备

- 设备：北通游戏手柄
- Windows 识别名称：`Controller (BEITONG A1S2 XINPUT GAMEPAD)`
- 输入协议：`XInput`

#### 3.3 技术栈

- C++20
- Qt 6.8.3
- Qt Widgets
- CMake
- Win32 API（Windows 接口）

### 4. MVP（最小可行产品）范围

#### 4.1 首版支持

- System Tray（系统托盘）常驻。
- 手动全局启停映射。
- `XInput` 手柄自动连接与断开重连。
- 鼠标指针移动。
- 鼠标左键与右键点击。
- 页面垂直滚动。
- 固定 Chrome Profile（Chrome 配置模板）。
- Chrome 常用快捷键。
- Modifier Layer（修饰层）。
- Dead Zone（死区）、鼠标灵敏度和滚动速度配置。
- 单项动作启停。
- Input Monitor（输入监视器）。
- 固定约 `125 Hz` 轮询频率，每约 `8 ms` 采样一次。

#### 4.2 首版不支持

- 任意改键。
- 宏录制与脚本。
- 文字输入。
- On-Screen Keyboard（屏幕键盘）。
- 开机自动启动。
- 多手柄同时控制。
- PlayStation、Switch Pro 与 DirectInput 手柄。
- 虚拟手柄驱动。
- UAC（用户账户控制）窗口操作。
- Chrome 前台进程自动检测。

### 5. 默认手柄布局

#### 5.1 基础层

| 手柄输入 | 动作 |
| --- | --- |
| `Right Stick` | 鼠标指针移动 |
| `Left Stick Up / Down` | 页面垂直滚动 |
| `A` | 鼠标左键 |
| `B` | 鼠标右键 |
| `X` | Chrome 后退：`Alt+Left` |
| `Y` | Chrome 前进：`Alt+Right` |
| `LB` | 按住进入 Navigation Layer |
| `RB` | 按住进入 Tab Layer |
| `View` | 启停映射 |

#### 5.2 Navigation Layer：按住 `LB`

| 手柄输入 | Chrome 动作 |
| --- | --- |
| `A` | 刷新：`Ctrl+R` |
| `B` | 停止加载：`Esc` |
| `X` | 页面顶部：`Home` |
| `Y` | 页面底部：`End` |
| `D-Pad Up` | 放大：`Ctrl++` |
| `D-Pad Down` | 缩小：`Ctrl+-` |
| `D-Pad Left` | 恢复 `100%`：`Ctrl+0` |
| `D-Pad Right` | 聚焦地址栏：`Ctrl+L` |

#### 5.3 Tab Layer：按住 `RB`

| 手柄输入 | Chrome 动作 |
| --- | --- |
| `A` | 新建标签页：`Ctrl+T` |
| `B` | 关闭当前标签页：`Ctrl+W` |
| `X` | 上一个标签页：`Ctrl+Shift+Tab` |
| `Y` | 下一个标签页：`Ctrl+Tab` |
| `D-Pad Left` | 恢复最近关闭的标签页：`Ctrl+Shift+T` |
| `D-Pad Right` | 页面内查找：`Ctrl+F` |

### 6. 已确认交互规则

- Modifier Layer（修饰层）仅在按住 `LB` 或 `RB` 时生效。
- 同时按住 `LB + RB` 时不执行层级动作，避免歧义。
- 快捷键动作仅在 Rising Edge（上升沿）触发一次，长按不会重复发送快捷键。
- 鼠标移动与页面滚动允许持续触发。
- 映射关闭时不发送任何键鼠事件，但 Input Monitor 仍显示设备输入。
- 关闭设置窗口只隐藏窗口，不退出后台映射。
- 通过托盘菜单显式退出应用。

## 设计第 2 部分：架构设计

### 7. 总体架构

采用“标准 C++20 Core + Win32 Adapter + Qt UI”方案。

```text
Qt Widgets UI
    -> Application Facade
        -> Mapping Engine
            -> IControllerInput
            -> IInputEmitter

XInputController implements IControllerInput
SendInputEmitter implements IInputEmitter
```

#### 7.1 Dependency Direction（依赖方向）

```text
Gui -> Application -> Core <- Platform/Windows
```

- `Gui` 模块可以使用 Qt。
- `Core` 模块尽可能仅依赖 C++20 Standard Library（标准库）。
- `Platform/Windows` 模块封装 Win32 API。
- UI 不直接调用 `XInput` 或 `SendInput`。
- Qt 类型不穿透到 `Core`。

#### 7.2 模块职责

| 模块 | 职责 | 允许依赖 |
| --- | --- | --- |
| `Gui` | 设置窗口、System Tray（系统托盘）、Input Monitor（输入监视器）、用户操作入口 | Qt Widgets、`Application` |
| `Application` | 组合 Core 与 Windows Adapter，提供面向 UI 的 Facade（门面） | `Core`、`Platform/Windows` |
| `Core` | Chrome Profile、Modifier Layer、按键边沿判断、动作生成、配置模型、状态快照 | C++20 Standard Library（标准库） |
| `Platform/Windows` | `XInput` 手柄采样、`SendInput` 键鼠注入 | Win32 API、`Core` 定义的接口 |

#### 7.3 关键接口

| 接口或组件 | 职责 |
| --- | --- |
| `IControllerInput` | 读取标准化后的手柄状态，不向 Core 暴露 `XINPUT_STATE` |
| `XInputController` | 使用 `XInput` 实现 `IControllerInput` |
| `MappingEngine` | 根据 Chrome Profile、当前 Layer 和按键边沿生成动作 |
| `IInputEmitter` | 接收 Core 动作并输出键盘或鼠标事件 |
| `SendInputEmitter` | 使用 `SendInput` 实现 `IInputEmitter` |
| `ApplicationFacade` | 管理启停、配置读写和 Input Monitor 所需的状态快照 |

#### 7.4 Ownership（所有权）

- Qt UI 对象使用 Qt `parent-child ownership（父子所有权）`。
- Core 对象使用 RAII（资源获取即初始化）和 `std::unique_ptr` 管理生命周期。

#### 7.5 Thread Affinity（线程亲和性）与 Event Model（事件模型）

- Qt 对象仅位于 GUI Thread（界面线程）。
- 手柄输入轮询运行于 `std::jthread`。
- 输入线程通过 immutable snapshot（不可变快照）发布状态。
- UI 定期读取 snapshot，不让 Qt signal-slot（信号槽）机制穿透 Core。

#### 7.6 数据流

```text
XInputController
    -> ControllerSnapshot
        -> MappingEngine
            -> std::variant<Action...>
                -> SendInputEmitter

MappingEngine
    -> MonitorSnapshot
        -> ApplicationFacade
            -> Qt Widgets UI 定期读取
```

- 手柄输入线程每约 `8 ms` 读取一次 `ControllerSnapshot`。
- `MappingEngine` 在输入线程中完成 Layer 选择、Dead Zone 处理和 Rising Edge 判断。
- 当映射处于启用状态时，动作交给 `SendInputEmitter` 串行发送。
- Input Monitor 仅读取 `MonitorSnapshot`，不直接访问输入线程内部对象。

#### 7.7 Trade-off（权衡）

- 首版使用 `XInput + SendInput`，无需开发驱动，足以覆盖已确认的北通手柄和 Chrome 浏览场景。
- `SendInput` 受到 UIPI（用户界面特权隔离）限制，不能可靠控制更高完整性级别的窗口。
- 首版不引入 Raw Input、SDL3 或虚拟手柄驱动，以控制开发与验证成本。

### 8. C++20 使用原则

C++20 特性用于解决实际工程问题，不为了展示语法而增加复杂度。

| C++20 特性 | 用途 |
| --- | --- |
| `std::jthread`、`std::stop_token` | 输入轮询线程自动停止与回收 |
| `std::chrono` | `8 ms` 采样周期与按键时间处理 |
| `std::variant` | 表达鼠标动作与快捷键动作 |
| `std::visit` | 分发动作到输出适配器 |
| `std::span` | 只读遍历固定按键表 |
| `std::optional` | 表达手柄未连接或无有效动作 |
| `std::ranges` | 过滤已启用的映射项 |
| `std::atomic` | 映射启停状态与低成本状态发布 |
| `concept` | 约束可替换的输入适配器 |
| Designated Initializer（指定初始化器） | 清晰构造默认 Chrome Profile |

## 设计第 3 部分：配置持久化与 UI

### 9. 配置持久化

#### 9.1 保存范围

退出应用后保留以下配置：

- Dead Zone（死区）。
- 鼠标灵敏度。
- 页面滚动速度。
- 单项动作启停状态。

#### 9.2 保存位置

配置文件保存到：

```text
%APPDATA%/PadNav/config.json
```

#### 9.3 JSON 实现边界

- 使用 Qt 的 `QJsonObject`、`QJsonDocument` 和 `QFile` 完成 JSON 读写。
- JSON 序列化属于 UI 边界层，不进入 `Core`。
- `Core` 仅暴露标准 C++ 配置结构，不依赖 Qt 类型。
- 首版不引入第三方 JSON library（JSON 库）。

#### 9.4 生效时机

- 设置窗口提供“应用”按钮。
- 用户点击“应用”后，修改后的配置一次性生效并写入 JSON 文件。
- 用户取消修改时，丢弃尚未应用的配置。

#### 9.5 异常恢复

- 配置文件缺失时，静默使用默认配置。
- 配置文件损坏或字段无法解析时，静默使用默认配置。
- 首版不弹出托盘提示。

### 10. 首版 UI

首版以功能可用为优先级，界面只提供必要入口。

#### 10.1 System Tray（系统托盘）

应用启动后常驻系统托盘。关闭设置窗口不会退出应用。

托盘菜单仅包含：

- 启用 / 暂停映射。
- 打开设置。
- 退出应用。

#### 10.2 设置窗口

设置窗口仅提供必要参数：

- Dead Zone（死区）。
- 鼠标灵敏度。
- 页面滚动速度。
- 单项动作启停。
- 打开 Input Monitor（输入监视器）的入口。
- “应用”和“取消”操作。

#### 10.3 Input Monitor（输入监视器）

- Input Monitor 是独立调试窗口。
- 默认不显示。
- 用户从设置窗口按需打开。
- 实时显示手柄连接状态、摇杆位置、按键状态、当前 Layer（层）和映射启停状态。

## 设计第 4 部分：异常处理与诊断

### 11. Debug 日志

- 普通日志仅在 Debug Build（调试构建）输出到控制台。
- Release Build（发布构建）不写本地滚动日志。
- 不记录每次摇杆采样，避免高频输出影响输入循环。

### 12. Windows Crash Dump（崩溃转储）

#### 12.1 转储类型

- 使用 `MiniDumpNormal`。
- 转储包含定位常见崩溃所需的线程、调用栈和基础模块信息。

#### 12.2 保存位置

```text
%LOCALAPPDATA%/PadNav/CrashDumps/
```

#### 12.3 保留策略

- 最多保留最近 `3` 个 dmp 文件。
- 应用启动时清理更旧的 dmp 文件。

#### 12.4 实现边界

- Crash Dump 实现位于 `Platform/Windows`。
- 使用 `SetUnhandledExceptionFilter` 捕获未处理异常。
- 使用 `MiniDumpWriteDump` 写入 dmp 文件。
- Windows 构建链接 `DbgHelp`。

## 设计第 5 部分：默认参数与验收标准

### 13. 默认参数

| 参数 | 默认值 | 允许范围 |
| --- | --- | --- |
| Dead Zone（死区） | `15%` | `0% - 40%` |
| 鼠标最大速度 | `1200 px/s` | `300 - 2400 px/s` |
| 页面滚动速度 | `720 wheel units/s` | `120 - 1440 wheel units/s` |

### 14. 自动化测试范围

首版自动化测试聚焦不依赖 Qt 和真实手柄的 `Core`。

必须覆盖：

- 默认 Chrome Profile 的动作映射。
- 基础层、Navigation Layer 和 Tab Layer 的切换。
- 同时按住 `LB + RB` 时不执行层级动作。
- 快捷键仅在 Rising Edge（上升沿）触发一次。
- 鼠标移动和页面滚动允许持续生成动作。
- 映射关闭时不向输出端发送动作。
- Dead Zone（死区）内的摇杆输入不生成移动或滚动动作。
- Dead Zone 外的摇杆输入生成移动或滚动动作。
- 单项动作关闭后不再生成对应动作。
- 默认配置值与参数允许范围。

### 15. Windows 手工验收范围

使用 Windows 11、Chrome 浏览器和北通 `BEITONG A1S2 XINPUT GAMEPAD` 执行手工验收。

#### 15.1 Chrome 连续浏览体验

- 连续浏览网页至少 `15` 分钟。
- 鼠标移动无明显卡顿、漂移或误触。
- 页面滚动连续且方向正确。
- 鼠标左键、右键操作正确。
- 基础层和 Modifier Layer（修饰层）切换符合默认布局。
- Chrome 常用快捷键均能正确执行。

#### 15.2 应用运行行为

- 应用启动后进入 System Tray（系统托盘）。
- 关闭设置窗口后，后台映射继续运行。
- 托盘菜单可以启用或暂停映射。
- 暂停映射后不发送任何键盘或鼠标事件。
- 托盘菜单可以重新打开设置窗口。
- 托盘菜单可以正常退出应用。

#### 15.3 手柄连接行为

- 手柄已连接时，应用可以正常读取输入。
- 运行期间断开手柄后，应用保持运行。
- 重新连接手柄后，输入功能自动恢复。

#### 15.4 配置行为

- 点击“应用”后，配置立即生效并写入 `%APPDATA%/PadNav/config.json`。
- 重启应用后，已保存配置仍然生效。
- 点击“取消”后，尚未应用的配置被丢弃。
- 配置文件缺失时，应用静默恢复默认配置。
- 配置文件损坏时，应用静默恢复默认配置。

#### 15.5 Input Monitor（输入监视器）

- 可以从设置窗口按需打开 Input Monitor。
- Input Monitor 可以显示连接状态、摇杆位置、按键状态、当前 Layer 和映射启停状态。
- 关闭 Input Monitor 不影响后台映射。

#### 15.6 Crash Dump（崩溃转储）

- 未处理异常发生后，在 `%LOCALAPPDATA%/PadNav/CrashDumps/` 生成 `MiniDumpNormal` dmp 文件。
- 应用启动时最多保留最近 `3` 个 dmp 文件。

## 附录 A：市场扫描结论

| 应用 | 公开能力 | 对 PadNav 的启发 |
| --- | --- | --- |
| [JoyToKey](https://joytokey.net/en/overview) | 手柄映射键盘、鼠标移动、滚轮、组合键；支持 Profile 切换 | 与 PadNav MVP 接近，但 PadNav 聚焦 Chrome 浏览体验 |
| [AntiMicroX](https://github.com/AntiMicroX/antimicrox) | 开源；支持键鼠映射、脚本、宏和 Auto Profile | 可参考能力边界，但首版不引入宏和脚本 |
| [reWASD](https://www.rewasd.com/advanced-controller-mapping) | 多设备、复杂宏、Shift Mode 和虚拟手柄 | 能力较重，超出 PadNav 首版范围 |
| [DS4Windows](https://ds4-windows.com/) | PlayStation 手柄适配、键鼠映射、虚拟设备和 Profile | 驱动与设备兼容成本较高，不作为首版方向 |
| [Steam Input](https://partner.steamgames.com/doc/features/steam_controller/concepts) | 将 Controller 输入映射为 Mouse、Keyboard 或 XInput | 能力完整，但依赖 Steam 生态 |

PadNav 的差异化方向：

> 面向 Chrome 浏览器与 Windows 桌面操作，提供开箱即用的手柄导航体验，同时保持轻量、无驱动和清晰的模块边界。

## 附录 B：Win32 API 参考

- [Microsoft XInput 文档](https://learn.microsoft.com/en-us/windows/win32/xinput/getting-started-with-xinput)
- [Microsoft SendInput 文档](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput)

