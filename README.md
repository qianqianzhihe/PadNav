# PadNav

副标题：用手柄，掌控桌面  
英文 Slogan：Turn your controller into a desktop navigator

PadNav（Gamepad Navigation）是一个面向 Windows 11 的轻量级手柄导航应用。首个版本围绕 Chrome 浏览器使用场景优化，目标是把常见的页面浏览、标签页切换、滚动、点击和快捷键操作，映射到 XInput 兼容手柄上。

项目命名逻辑：
- `Pad`：手柄
- `Nav`：导航 / Navigate

PadNav 首版不是通用改键器，也不追求覆盖所有应用场景。它更像一个专注于桌面浏览体验的常驻工具：启动后以系统托盘方式运行，通过固定的 Chrome Profile 提供稳定、低学习成本的操作布局。

## 产品介绍

PadNav MVP 的核心定位是：
- 面向 Windows 11
- 优先支持 Chrome 浏览器
- 优先支持 XInput 手柄
- 以托盘常驻方式运行
- 提供“开箱即用”的默认手柄导航布局

适合的使用场景包括：
- 在沙发、电视或远离键鼠的环境中浏览网页
- 用手柄完成页面前进后退、滚动和点击
- 快速切换标签页、刷新页面、恢复已关闭标签页
- 在不打开复杂设置系统的前提下，直接获得稳定的浏览器导航体验

首版明确不做：
- 任意改键
- 宏录制与脚本
- 多手柄同时控制
- PlayStation / Switch Pro / DirectInput 手柄支持
- 开机自启动
- UAC 窗口控制

## 核心能力

- System Tray（系统托盘）常驻
- 手动启停映射
- `XInput` 手柄自动连接与断开重连
- 鼠标指针移动
- 鼠标左右键点击
- 页面垂直滚动
- Chrome 快捷键映射
- `LB` / `RB` 修饰层
- Dead Zone、鼠标灵敏度、滚动速度配置
- 单项动作启停
- Input Monitor（输入监视器）

## 默认快捷键说明

PadNav 默认使用固定的 Chrome Profile。基础层负责常用浏览动作，`LB` 和 `RB` 作为按住生效的修饰层，用来切换到页面导航和标签页操作。

### 基础层

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

### Navigation Layer：按住 `LB`

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

### Tab Layer：按住 `RB`

| 手柄输入 | Chrome 动作 |
| --- | --- |
| `A` | 新建标签页：`Ctrl+T` |
| `B` | 关闭当前标签页：`Ctrl+W` |
| `X` | 上一个标签页：`Ctrl+Shift+Tab` |
| `Y` | 下一个标签页：`Ctrl+Tab` |
| `D-Pad Left` | 恢复最近关闭的标签页：`Ctrl+Shift+T` |
| `D-Pad Right` | 页面内查找：`Ctrl+F` |

## 交互规则

- 修饰层只在按住 `LB` 或 `RB` 时生效
- 同时按住 `LB + RB` 时不执行层级动作，避免歧义
- 快捷键动作只在按键上升沿触发一次，长按不会持续重复发送
- 鼠标移动和页面滚动允许持续触发
- 映射关闭时不发送键鼠事件，但 Input Monitor 仍可显示设备输入
- 关闭设置窗口只隐藏窗口，不退出后台映射
- 退出应用通过托盘菜单显式执行

## 当前运行环境

- 操作系统：Windows 11
- 手柄设备：北通游戏手柄
- Windows 识别名称：`Controller (BEITONG A1S2 XINPUT GAMEPAD)`
- 输入协议：`XInput`
- 技术栈：C++20、Qt 6.8.3、Qt Widgets、CMake、Win32 API

## 相关文档

- 需求文档：[Doc/Requirements/PadNav-MVP-Requirements.md](Doc/Requirements/PadNav-MVP-Requirements.md)
- 实施计划：[Doc/Plans/2026-05-30-padnav-mvp-implementation-plan.md](Doc/Plans/2026-05-30-padnav-mvp-implementation-plan.md)
- 阶段路线图：[Doc/Plans/2026-06-06-padnav-mvp-phase-roadmap.md](Doc/Plans/2026-06-06-padnav-mvp-phase-roadmap.md)
