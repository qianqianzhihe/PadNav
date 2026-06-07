# PadNav MVP 实现计划

> **给 agentic workers（代理式执行者）的要求：** REQUIRED SUB-SKILL（必需子技能）：按任务逐项实现本计划时，使用 `superpowers:subagent-driven-development`（推荐）或 `superpowers:executing-plans`。步骤使用 checkbox（复选框）语法（`- [ ]`）进行跟踪。

**目标：** 构建一个 Windows 11 System Tray（系统托盘）应用，将 XInput 兼容的北通手柄映射为面向 Chrome 的鼠标、滚动和键盘快捷键动作。

**架构：** 将映射领域逻辑保持在不依赖 Qt 的 C++20 `Core` library（核心库）中。将 XInput、SendInput 和 Crash Dump（崩溃转储）处理放在 Win32 adapter library（适配库）中；将综合管理类 `PadNavController` 放在 `Application` 中；将 Qt Widgets 和 Qt JSON 持久化限定在 `Gui` 中。开发前期用 `main.cpp + QTimer + PadNavController::pollOnce()` 建立可观察调试闭环，稳定后再收敛为 `std::jthread` 后台轮询。

**技术栈：** C++20、Qt 6.8.3 Widgets、modern CMake、Win32 XInput、SendInput、DbgHelp、CTest。

---

## 1. 范围与交付顺序

每次只实现一个可观察、可调试的纵向切片：

1. CMake target、alias 和 `padnav_tests` 构建基线。
2. `PadNavController::pollOnce()` 调试壳，不加线程。
3. `XInputController` 读取真实北通手柄，并在临时 UI 或控制台显示状态。
4. `SendInputEmitter` 单独验证鼠标、滚轮和少量快捷键注入。
5. `Core` 模型和 `MappingEngine` 接入真实输入输出闭环。
6. Qt JSON repository（仓储）、SettingsWindow 和 Input Monitor。
7. 将 `QTimer` 调试轮询收敛到 `PadNavController::start()` / `stop()` 和 `std::jthread`。
8. Windows Crash Dump bootstrap（崩溃转储启动逻辑）与完整应用组装。
9. 自动化验证和 Windows 11 手工验收。

不要加入任意改键、宏、脚本、开机启动、多手柄支持、前台进程检测、虚拟手柄或非 XInput 手柄 adapter。

`PadNavController` 是 main.cpp 创建并持有的 single owner（单一拥有者），不是硬 Singleton（单例）。UI 通过引用使用它，不通过 `static instance()` 全局访问。

## 2. 文件结构

### Core：仅使用标准 C++20

| 文件 | 职责 |
| --- | --- |
| `Src/Core/Interface/PadNav/CMakeLists.txt` | 构建 `Pn::PadNavCore`。 |
| `Src/Core/Interface/PadNav/controllerstate.h` | 标准化后的手柄按键、摇杆和状态快照。 |
| `Src/Core/Interface/PadNav/action.h` | 面向鼠标、滚动和键盘输出的 `std::variant` 动作模型。 |
| `Src/Core/Interface/PadNav/profile.h` | Chrome action IDs（动作 ID）、启停标志、参数范围和默认 profile（配置模板）。 |
| `Src/Core/Interface/PadNav/profile.cpp` | 默认 Chrome profile 和校验逻辑。 |
| `Src/Core/Interface/PadNav/mappingengine.h` | 有状态映射 API。 |
| `Src/Core/Interface/PadNav/mappingengine.cpp` | Dead Zone（死区）处理、Layer（层）、Rising Edge（上升沿）和持续动作。 |

### Platform：仅 Win32

本项目只适配 Windows 平台，因此不再保留 `Src/Platform` 聚合模块；`WindowsPlatform` 直接位于 `Src/Core/Interface/WindowsPlatform`。

| 文件 | 职责 |
| --- | --- |
| `Src/Core/Interface/WindowsPlatform/CMakeLists.txt` | 构建 `Pn::WindowsPlatform`；链接 `XInput` 和 `DbgHelp`。 |
| `Src/Core/Interface/WindowsPlatform/xinputcontroller.h/.cpp` | 读取 controller `0`，标准化摇杆，并自动重连。 |
| `Src/Core/Interface/WindowsPlatform/sendinputemitter.h/.cpp` | 将 Core 动作转换为有序的 `SendInput` 事件。 |
| `Src/Core/Interface/WindowsPlatform/crash_dump.h/.cpp` | 安装未处理异常过滤器，并保留最近三个 dmp。 |

### Application：标准 C++20 编排层

| 文件 | 职责 |
| --- | --- |
| `Src/Core/Interface/Application/CMakeLists.txt` | 构建 `Pn::Application`。 |
| `Src/Core/Interface/Application/padnavcontroller.h/.cpp` | 综合管理类；拥有 adapters，提供 `pollOnce()` 调试入口，后期运行 `8 ms` 后台轮询，发布 monitor snapshots（监视快照），应用 profiles。 |

### Gui：Qt 边界

| 文件 | 职责 |
| --- | --- |
| `Src/Gui/Scene/PadNav/CMakeLists.txt` | 构建 `Pn::PadNavGui`。 |
| `Src/Gui/Scene/PadNav/json_profile_repository.h/.cpp` | 使用 Qt JSON 读写 `%APPDATA%/PadNav/config.json`。 |
| `Src/Gui/Scene/PadNav/settings_window.h/.cpp` | 最小设置表单，包含 Apply、Cancel 和 Input Monitor 入口。 |
| `Src/Gui/Scene/PadNav/input_monitor_window.h/.cpp` | 轮询 immutable snapshots（不可变快照）并渲染设备状态。 |
| `Src/Gui/Scene/PadNav/tray_controller.h/.cpp` | 托盘图标和三个已确认菜单入口。 |

### App 和 tests

| 文件 | 职责 |
| --- | --- |
| `Src/App/main.cpp` | 组合 repositories、adapters、controller、crash dumps、tray 和 windows。 |
| `Tests/PnTests/CMakeLists.txt` | 构建 `padnav_tests`。 |
| `Tests/PnTests/main.cpp` | 测试入口。 |
| `Tests/PnTests/PadNavCore/profiletests.cpp` | 校验默认值和参数范围。 |
| `Tests/PnTests/PadNavCore/mappingenginetests.cpp` | 校验 layers、rising edges、dead zone、enable flags 和 pause 行为。 |
| `Doc/Verification/PadNav-MVP-Windows-Manual-Checklist.md` | Windows 11 手工验收 checklist（检查清单）。 |

## 3. Dependency Direction（依赖方向）

```text
PadNav executable
    -> Pn::PadNavGui
    -> Pn::Application
        -> Pn::PadNavCore
        -> Pn::WindowsPlatform
            -> Pn::PadNavCore

Pn::PadNavGui
    -> Qt6::Core
    -> Qt6::Widgets
    -> Pn::Application
    -> Pn::PadNavCore
```

`Pn::PadNavCore`、`Pn::Application` 和 `Pn::WindowsPlatform` 不允许 include Qt headers（Qt 头文件）。`QJsonObject`、`QJsonDocument`、`QFile`、`QWidget` 和 `QSystemTrayIcon` 必须留在 `Src/Gui` 内。

## 4. 任务拆分

说明：本节任务是文件级工作包，不强制代表人工开发的先后顺序。人工实现时优先按 `Doc/Plans/2026-06-06-padnav-mvp-phase-roadmap.md` 的 Vertical Slice（纵向切片）阶段推进：先 `main.cpp + QTimer + PadNavController::pollOnce()`，再接 XInput、SendInput、MappingEngine、UI、JSON，最后收敛到 `std::jthread`。

### 任务 1：建立 Target-Based CMake 拓扑和不依赖 Qt 的 Core Test Harness（核心测试框架）

**文件：**
- 修改: `CMakeLists.txt`
- 修改: `Src/CMakeLists.txt`
- 修改: `Src/Core/CMakeLists.txt`
- 修改: `Src/Core/Interface/CMakeLists.txt`
- 新建: `Src/Core/Interface/PadNav/CMakeLists.txt`
- 新建: `Src/Core/Interface/WindowsPlatform/CMakeLists.txt`
- 新建: `Src/Core/Interface/Application/CMakeLists.txt`
- 修改: `Src/Gui/CMakeLists.txt`
- 修改: `Src/Gui/Scene/CMakeLists.txt`
- 新建: `Src/Gui/Scene/PadNav/CMakeLists.txt`
- 修改: `Tests/CMakeLists.txt`
- 修改: `Tests/PnTests/CMakeLists.txt`
- 修改: `Tests/PnTests/main.cpp`

- [ ] **步骤 1：添加一个会失败的 Core test executable（核心测试可执行程序）**

修改 `Tests/PnTests/main.cpp`：

```cpp
#include <cstdlib>
#include <iostream>

int main() {
    std::cout << "padnav_tests bootstrap\n";
    return EXIT_SUCCESS;
}
```

修改 `Tests/PnTests/CMakeLists.txt`：

```cmake
add_executable(padnav_tests
    main.cpp
)

target_link_libraries(padnav_tests PRIVATE
    Pn::PadNavCore
)

add_test(NAME padnav_tests COMMAND padnav_tests)
```

修改 `Tests/CMakeLists.txt`：

```cmake
add_subdirectory(PnTests)
```

- [ ] **步骤 2：配置工程，验证缺失 target 会失败**

运行：

```powershell
cmake -S . -B cmake-build-debug
```

预期: configuration 失败，因为 `Pn::PadNavCore` 不存在。

- [ ] **步骤 3：添加 target topology（目标拓扑）**

在根目录 `CMakeLists.txt` 的 `add_subdirectory(Src)` 之前追加：

```cmake
include(CTest)
```

替换 `Src/CMakeLists.txt`：

```cmake
add_subdirectory(Core)
add_subdirectory(Gui)
add_subdirectory(App)
```

确认 `Src/Core/CMakeLists.txt` 包含：

```cmake
add_subdirectory(Interface)
```

新建 `Src/Core/Interface/PadNav/CMakeLists.txt`：

```cmake
add_library(pn_pad_nav_core INTERFACE)
add_library(Pn::PadNavCore ALIAS pn_pad_nav_core)

target_include_directories(pn_pad_nav_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
```

修改 `Src/Core/Interface/CMakeLists.txt`：

```cmake
add_subdirectory(Application)
add_subdirectory(PadNav)
add_subdirectory(WindowsPlatform)
```

新建 `Src/Core/Interface/WindowsPlatform/CMakeLists.txt`：

```cmake
add_library(pn_windows_platform INTERFACE)
add_library(Pn::WindowsPlatform ALIAS pn_windows_platform)

target_include_directories(pn_windows_platform INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(pn_windows_platform INTERFACE
    Pn::PadNavCore
)
```

新建 `Src/Core/Interface/Application/CMakeLists.txt`：

```cmake
add_library(pn_application INTERFACE)
add_library(Pn::Application ALIAS pn_application)

target_include_directories(pn_application INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(pn_application INTERFACE
    Pn::PadNavCore
    Pn::WindowsPlatform
)
```

确认 `Src/Gui/CMakeLists.txt` 包含：

```cmake
add_subdirectory(Scene)
```

追加到 `Src/Gui/Scene/CMakeLists.txt`：

```cmake
add_subdirectory(PadNav)
```

新建 `Src/Gui/Scene/PadNav/CMakeLists.txt`：

```cmake
add_library(pn_pad_nav_gui INTERFACE)
add_library(Pn::PadNavGui ALIAS pn_pad_nav_gui)

target_include_directories(pn_pad_nav_gui INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(pn_pad_nav_gui INTERFACE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Pn::Application
    Pn::PadNavCore
)
```

- [ ] **步骤 4：配置、构建并运行 bootstrap test（引导测试）**

运行：

```powershell
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target padnav_tests
ctest --test-dir cmake-build-debug -R padnav_tests --output-on-failure
```

预期: configuration 成功，`padnav_tests` 通过。

- [ ] **步骤 5：提交拓扑结构**

```powershell
git add CMakeLists.txt Src Tests
git commit -m "build: add PadNav module topology"
```

### 任务 2：定义 Core Domain Model（核心领域模型）和默认 Chrome Profile

**文件：**
- 新建: `Src/Core/Interface/PadNav/controllerstate.h`
- 新建: `Src/Core/Interface/PadNav/action.h`
- 新建: `Src/Core/Interface/PadNav/profile.h`
- 新建: `Src/Core/Interface/PadNav/profile.cpp`
- 修改: `Src/Core/Interface/PadNav/CMakeLists.txt`
- 新建: `Tests/PnTests/PadNavCore/testsupport.h`
- 新建: `Tests/PnTests/PadNavCore/profiletests.cpp`
- 修改: `Tests/PnTests/main.cpp`
- 修改: `Tests/PnTests/CMakeLists.txt`

- [ ] **步骤 1：编写失败的 profile tests（配置测试）**

新建 `Tests/PnTests/PadNavCore/testsupport.h`：

```cpp
#pragma once

#include <stdexcept>
#include <string_view>

inline void require(bool condition, std::string_view message) {
    if (!condition) {
        throw std::runtime_error(message.data());
    }
}
```

新建 `Tests/PnTests/PadNavCore/profiletests.cpp`：

```cpp
#include "PadNav/profile.h"
#include "testsupport.h"

void runProfileTests() {
    const auto profile = pn::core::makeDefaultChromeProfile();
    require(profile.deadZonePercent == 15, "default dead zone");
    require(profile.mouseMaxSpeedPixelsPerSecond == 1200, "default mouse speed");
    require(profile.scrollSpeedWheelUnitsPerSecond == 720, "default scroll speed");
    require(profile.bindings.size() == 16, "chrome shortcut count");

    auto invalid = profile;
    invalid.deadZonePercent = 41;
    require(!pn::core::isValid(invalid), "dead zone upper bound");
}
```

替换 `Tests/PnTests/main.cpp`：

```cpp
#include <cstdlib>
#include <exception>
#include <iostream>

void runProfileTests();

int main() {
    try {
        runProfileTests();
        std::cout << "padnav_tests passed\n";
        return EXIT_SUCCESS;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return EXIT_FAILURE;
    }
}
```

- [ ] **步骤 2：运行测试，验证它们会失败**

运行：

```powershell
cmake --build cmake-build-debug --target padnav_tests
```

预期: compile 失败，因为 `PadNav/profile.h` 不存在。

- [ ] **步骤 3：添加完整 Core value types（核心值类型）**

新建 `Src/Core/Interface/PadNav/controllerstate.h`，内容为：

```cpp
#pragma once

#include <cstdint>

namespace pn::core {

enum class ControllerButton : std::uint16_t {
    A = 1U << 0U, B = 1U << 1U, X = 1U << 2U, Y = 1U << 3U,
    LeftShoulder = 1U << 4U, RightShoulder = 1U << 5U,
    View = 1U << 6U, DPadUp = 1U << 7U, DPadDown = 1U << 8U,
    DPadLeft = 1U << 9U, DPadRight = 1U << 10U
};

struct Stick final {
    float x{};
    float y{};
};

struct ControllerSnapshot final {
    bool connected{};
    std::uint16_t buttons{};
    Stick leftStick{};
    Stick rightStick{};
};

constexpr auto mask(ControllerButton button) -> std::uint16_t {
    return static_cast<std::uint16_t>(button);
}

constexpr auto isPressed(const ControllerSnapshot& state, ControllerButton button) -> bool {
    return (state.buttons & mask(button)) != 0U;
}

}  // namespace pn::core
```

新建 `Src/Core/Interface/PadNav/action.h`，包含 `KeyCode`、`Modifier`、`MouseMove`、`MouseButtonAction`、`Scroll`、`KeyChord`，以及：

```cpp
using Action = std::variant<MouseMove, MouseButtonAction, Scroll, KeyChord>;
```

使用整数鼠标位移、整数 wheel units、`MouseButton::{Left, Right}`、`ButtonTransition::{Press, Release}`，以及 profile 需要的 key codes：`Left`、`Right`、`R`、`Escape`、`Home`、`End`、`Plus`、`Minus`、`Digit0`、`L`、`T`、`W`、`Tab`、`F`。

新建 `Src/Core/Interface/PadNav/profile.h`，内容为：

```cpp
enum class ChromeActionId {
    Back, Forward, Reload, StopLoading, PageTop, PageBottom, ZoomIn, ZoomOut,
    ResetZoom, FocusAddressBar, NewTab, CloseTab, PreviousTab, NextTab,
    RestoreClosedTab, FindInPage
};

struct ActionBinding final {
    ChromeActionId id{};
    bool enabled{true};
};

struct ChromeProfile final {
    int deadZonePercent{15};
    int mouseMaxSpeedPixelsPerSecond{1200};
    int scrollSpeedWheelUnitsPerSecond{720};
    std::array<ActionBinding, 16> bindings{};
};

auto makeDefaultChromeProfile() -> ChromeProfile;
auto isValid(const ChromeProfile& profile) -> bool;
auto isEnabled(const ChromeProfile& profile, ChromeActionId id) -> bool;
```

使用 Designated Initializer（指定初始化器）和 `std::ranges::find_if` 实现 `profile.cpp`。校验规则：

```text
deadZonePercent: 0..40
mouseMaxSpeedPixelsPerSecond: 300..2400
scrollSpeedWheelUnitsPerSecond: 120..1440
```

- [ ] **步骤 4：将 Core 转为 static library（静态库），注册 sources，并通过 profile tests**

替换 `Src/Core/Interface/PadNav/CMakeLists.txt`：

```cmake
add_library(pn_pad_nav_core STATIC)
add_library(Pn::PadNavCore ALIAS pn_pad_nav_core)

target_sources(pn_pad_nav_core PRIVATE
    profile.cpp
)

target_include_directories(pn_pad_nav_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
```

将 `profiletests.cpp` 加入 `Tests/PnTests/CMakeLists.txt`。

运行：

```powershell
cmake --build cmake-build-debug --target padnav_tests
ctest --test-dir cmake-build-debug -R padnav_tests --output-on-failure
```

预期: profile tests 通过。

- [ ] **步骤 5：提交 Core values**

```powershell
git add Src/Core/Interface/PadNav Tests/PnTests
git commit -m "feat: add PadNav core profile model"
```

### 任务 3：实现带 Layer 和 Edge Tests 的 MappingEngine

**文件：**
- 新建: `Src/Core/Interface/PadNav/mappingengine.h`
- 新建: `Src/Core/Interface/PadNav/mappingengine.cpp`
- 修改: `Src/Core/Interface/PadNav/CMakeLists.txt`
- 新建: `Tests/PnTests/PadNavCore/mappingenginetests.cpp`
- 修改: `Tests/PnTests/main.cpp`
- 修改: `Tests/PnTests/CMakeLists.txt`

- [ ] **步骤 1：编写失败的 mapping tests（映射测试）**

新建 `Tests/PnTests/PadNavCore/mappingenginetests.cpp`，测试用例调用：

```cpp
pn::core::MappingEngine engine;
auto profile = pn::core::makeDefaultChromeProfile();
engine.setProfile(profile);
engine.setMappingEnabled(true);
const auto actions = engine.map(snapshot, std::chrono::milliseconds{8});
```

断言以下精确行为：

```text
A rising edge -> MouseButtonAction{Left, Press}
A falling edge -> MouseButtonAction{Left, Release}
X rising edge -> KeyChord{Alt, Left}
hold X for second sample -> no KeyChord
LB + A rising edge -> KeyChord{Control, R}
RB + A rising edge -> KeyChord{Control, T}
LB + RB + A rising edge -> no layer action
right stick within 15% dead zone -> no MouseMove
right stick outside dead zone -> MouseMove exists
left stick outside dead zone -> Scroll exists
mapping disabled -> no output actions
disabled Reload binding + LB + A -> no KeyChord
```

使用 `std::holds_alternative`、`std::get_if` 和小型 local helpers（本地辅助函数）保持每个断言清晰。

- [ ] **步骤 2：运行测试，验证缺失 engine 会失败**

运行：

```powershell
cmake --build cmake-build-debug --target padnav_tests
```

预期: compile 失败，因为 `PadNav/mappingengine.h` 不存在。

- [ ] **步骤 3：添加 engine API**

新建 `Src/Core/Interface/PadNav/mappingengine.h`：

```cpp
#pragma once

#include <chrono>
#include <vector>

#include "PadNav/action.h"
#include "PadNav/controllerstate.h"
#include "PadNav/profile.h"

namespace pn::core {

enum class ActiveLayer { Base, Navigation, Tab, Ambiguous };

struct MappingResult final {
    ActiveLayer activeLayer{ActiveLayer::Base};
    std::vector<Action> actions;
};

class MappingEngine final {
public:
    void setProfile(ChromeProfile profile);
    void setMappingEnabled(bool enabled);
    [[nodiscard]] auto map(const ControllerSnapshot& current,
                           std::chrono::milliseconds elapsed) -> MappingResult;

private:
    ChromeProfile m_profile{makeDefaultChromeProfile()};
    ControllerSnapshot m_previous{};
    bool m_mappingEnabled{true};
};

}  // namespace pn::core
```

- [ ] **步骤 4：实现确定性的 mapping（映射）逻辑**

在 `mappingengine.cpp` 中：

1. 根据肩键选择 `ActiveLayer`。
2. 对两个摇杆应用 radial dead-zone scaling（径向死区缩放）。
3. 根据缩放后的摇杆和 elapsed time（经过时间）生成持续 `MouseMove` 和 `Scroll`。
4. 为基础层 `A` 和 `B` 生成鼠标 press/release transitions（按下/释放转换）。
5. 快捷键 chord（组合键）仅在 rising edge 生成。
6. 使用固定 `std::array` mapping table（映射表），每层使用 `std::span`。
7. 通过 `isEnabled` 跳过禁用的 `ChromeActionId`。
8. 返回前执行 `m_previous = current`。

使用 `std::variant` actions，并让 Windows virtual-key values（虚拟键值）保持在 Core 之外。

- [ ] **步骤 5：通过所有 Core tests**

将 `mappingengine.cpp` 和 `mappingenginetests.cpp` 加入对应 targets。向 test runner 加入 `runMappingEngineTests()`。

运行：

```powershell
cmake --build cmake-build-debug --target padnav_tests
ctest --test-dir cmake-build-debug -R padnav_tests --output-on-failure
```

预期: 所有 Core tests 通过。

- [ ] **步骤 6：提交 mapping engine**

```powershell
git add Src/Core/Interface/PadNav Tests/PnTests
git commit -m "feat: add Chrome controller mapping engine"
```

### 任务 4：添加 Win32 XInput 和 SendInput Adapters

**文件：**
- 新建: `Src/Core/Interface/WindowsPlatform/xinputcontroller.h`
- 新建: `Src/Core/Interface/WindowsPlatform/xinputcontroller.cpp`
- 新建: `Src/Core/Interface/WindowsPlatform/sendinputemitter.h`
- 新建: `Src/Core/Interface/WindowsPlatform/sendinputemitter.cpp`
- 修改: `Src/Core/Interface/WindowsPlatform/CMakeLists.txt`

- [ ] **步骤 1：添加 XInput adapter**

新建 `xinputcontroller.h`：

```cpp
class XInputController final {
public:
    [[nodiscard]] auto poll() const -> pn::core::ControllerSnapshot;
};
```

使用 `XInputGetState(0, &state)` 实现 `poll()`。任何非 `ERROR_SUCCESS` 的结果都返回 `{.connected = false}`。将 `SHORT` 摇杆值标准化到 `[-1.0F, 1.0F]`，并将 XInput buttons 映射为 `ControllerButton` bits，不向外暴露 `XINPUT_STATE`。

- [ ] **步骤 2：添加 SendInput adapter**

新建 `sendinputemitter.h`：

```cpp
class SendInputEmitter final {
public:
    auto emit(const pn::core::Action& action) const -> bool;
};
```

使用 `std::visit` 实现 `emit()`：

- `MouseMove` -> 一个 relative（相对）`INPUT_MOUSE` 事件。
- `Scroll` -> 一个 `MOUSEEVENTF_WHEEL` 事件。
- `MouseButtonAction` -> 一个左键或右键 press/release 事件。
- `KeyChord` -> modifier key-down events、key-down、key-up、反向 modifier key-up events。

在 `sendinputemitter.cpp` 的 private helper functions（私有辅助函数）中，将 Core `KeyCode` 和 `Modifier` 映射到 Win32 virtual keys。

- [ ] **步骤 3：链接 Win32 libraries 并构建 adapters**

替换 `Src/Core/Interface/WindowsPlatform/CMakeLists.txt`：

```cmake
add_library(pn_windows_platform STATIC)
add_library(Pn::WindowsPlatform ALIAS pn_windows_platform)

target_sources(pn_windows_platform PRIVATE
    xinputcontroller.cpp
    sendinputemitter.cpp
)

target_include_directories(pn_windows_platform PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(pn_windows_platform PUBLIC
    Pn::PadNavCore
    XInput
)
```

运行：

```powershell
cmake --build cmake-build-debug --target pn_windows_platform
```

预期: `pn_windows_platform` 构建成功。

- [ ] **步骤 4：提交 Win32 input 和 output**

```powershell
git add Src/Core/Interface/WindowsPlatform
git commit -m "feat: add XInput and SendInput adapters"
```

### 任务 5：添加 PadNavController 控制服务和 Immutable Monitor Snapshot

**文件：**
- 新建: `Src/Core/Interface/Application/padnavcontroller.h`
- 新建: `Src/Core/Interface/Application/padnavcontroller.cpp`
- 修改: `Src/Core/Interface/Application/CMakeLists.txt`

- [ ] **步骤 1：定义标准 C++20 controller API（控制 API）**

新建 `padnavcontroller.h`，内容为：

```cpp
struct MonitorSnapshot final {
    pn::core::ControllerSnapshot controller;
    pn::core::ActiveLayer activeLayer{pn::core::ActiveLayer::Base};
    bool mappingEnabled{true};
};

class PadNavController final {
public:
    PadNavController();
    ~PadNavController();
    PadNavController(const PadNavController&) = delete;
    auto operator=(const PadNavController&) -> PadNavController& = delete;

    void pollOnce();
    void start();
    void stop();
    void setMappingEnabled(bool enabled);
    [[nodiscard]] auto mappingEnabled() const -> bool;
    void applyProfile(const pn::core::ChromeProfile& profile);
    [[nodiscard]] auto profile() const -> pn::core::ChromeProfile;
    [[nodiscard]] auto monitorSnapshot() const -> MonitorSnapshot;

private:
    void run(std::stop_token stopToken);
};
```

使用 private implementation object（私有实现对象）隐藏同步细节，避免同步实现暴露在 header 中。开发前期只要求 `pollOnce()` 可用；`start()` / `stop()` 可以先保留空实现，等纵向调试闭环稳定后再接 `std::jthread`。

- [ ] **步骤 2：实现 ownership（所有权）和 `pollOnce()` 调试闭环**

在 `padnavcontroller.cpp` 中：

- 拥有 `XInputController`、`SendInputEmitter` 和 `MappingEngine`。
- 将 profile 和 `MonitorSnapshot` 存在 `std::mutex` 保护之下。
- 将 mapping enabled state 存在 `std::atomic_bool` 中。
- `pollOnce()` 执行一次完整链路：读取手柄、处理 `View` 启停、调用映射、串行注入动作、更新 `MonitorSnapshot`。
- 在 controller 中跟踪 previous controller snapshot（上一帧手柄快照）。当 `View` 从 released 变为 pressed 时只切换一次 mapping，即使当前 mapping 已暂停也要能切换。
- 映射前将最新 profile 应用到 engine。
- 使用 `SendInputEmitter` 串行发送每一个返回动作。
- 每次采样都更新 `MonitorSnapshot`，即使 mapping 已暂停。
- 在 `_DEBUG` 下，将 controller connect/disconnect transitions（连接/断开变化）和失败的 `SendInputEmitter::emit` 调用写入 `std::clog`。不要记录每次采样。

- [ ] **步骤 3：在 main.cpp 用 QTimer 临时驱动 `pollOnce()`**

在 `Src/App/main.cpp` 的调试阶段创建唯一 `PadNavController` 实例，并用 `QTimer` 驱动：

```cpp
pn::application::PadNavController controller;

QTimer pollTimer;
QObject::connect(&pollTimer, &QTimer::timeout, [&controller]() {
    controller.pollOnce();
});
pollTimer.start(8);
```

这个阶段允许先用 `qDebug()` 或临时 Input Monitor 观察 `monitorSnapshot()`。不要把 `QTimer` 写进 Core、WindowsPlatform 或最终的后台线程逻辑里。

- [ ] **步骤 4：构建 Application**

替换 `Src/Core/Interface/Application/CMakeLists.txt`：

```cmake
add_library(pn_application STATIC)
add_library(Pn::Application ALIAS pn_application)

target_sources(pn_application PRIVATE
    padnavcontroller.cpp
)

target_include_directories(pn_application PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(pn_application PUBLIC
    Pn::PadNavCore
    Pn::WindowsPlatform
)
```

运行：

```powershell
cmake --build cmake-build-debug --target pn_application
```

预期: `pn_application` 构建成功。

- [ ] **步骤 5：提交 controller service**

```powershell
git add Src/Core/Interface/Application
git commit -m "feat: add PadNav controller service"
```

### 任务 6：添加 Qt JSON Profile Repository

**文件：**
- 新建: `Src/Gui/Scene/PadNav/json_profile_repository.h`
- 新建: `Src/Gui/Scene/PadNav/json_profile_repository.cpp`
- 修改: `Src/Gui/Scene/PadNav/CMakeLists.txt`

- [ ] **步骤 1：定义 repository behavior（仓储行为）**

新建 `json_profile_repository.h`：

```cpp
class JsonProfileRepository final {
public:
    [[nodiscard]] auto load() const -> pn::core::ChromeProfile;
    auto save(const pn::core::ChromeProfile& profile) const -> bool;

private:
    [[nodiscard]] auto filePath() const -> QString;
};
```

- [ ] **步骤 2：实现仅 Qt 边界内的 JSON 持久化**

在 `json_profile_repository.cpp` 中：

- 使用 `QStandardPaths::AppDataLocation` 构建 `%APPDATA%/PadNav/config.json`。
- 使用 `QDir::mkpath`。
- 序列化 `deadZonePercent`、`mouseMaxSpeedPixelsPerSecond`、`scrollSpeedWheelUnitsPerSecond` 和全部 `ChromeActionId` enabled flags。
- 使用 `QJsonDocument` 和 `QJsonObject` 解析。
- 转换为 `pn::core::ChromeProfile`。
- 当文件不存在、格式错误、不完整或未通过 `isValid` 时，返回 `makeDefaultChromeProfile()`。
- 使用 `QSaveFile` 原子写入。
- 在 `QT_DEBUG` 下，使用 `qDebug()` 输出格式错误或无效配置导致 fallback（回退）的事件。不要创建日志文件。

- [ ] **步骤 3：将 Gui target 转为 static library 并构建 repository**

替换 `Src/Gui/Scene/PadNav/CMakeLists.txt`：

```cmake
add_library(pn_pad_nav_gui STATIC)
add_library(Pn::PadNavGui ALIAS pn_pad_nav_gui)

target_sources(pn_pad_nav_gui PRIVATE
    json_profile_repository.cpp
)

target_include_directories(pn_pad_nav_gui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(pn_pad_nav_gui PUBLIC
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Pn::Application
    Pn::PadNavCore
)
```

运行：

```powershell
cmake --build cmake-build-debug --target pn_pad_nav_gui
```

预期: `pn_pad_nav_gui` 构建成功。

- [ ] **步骤 4：提交 persistence（持久化）**

```powershell
git add Src/Gui/Scene/PadNav
git commit -m "feat: persist PadNav profile with Qt JSON"
```

### 任务 7：构建最小 Settings、Monitor 和 Tray UI

**文件：**
- 新建: `Src/Gui/Scene/PadNav/settings_window.h`
- 新建: `Src/Gui/Scene/PadNav/settings_window.cpp`
- 新建: `Src/Gui/Scene/PadNav/input_monitor_window.h`
- 新建: `Src/Gui/Scene/PadNav/input_monitor_window.cpp`
- 新建: `Src/Gui/Scene/PadNav/tray_controller.h`
- 新建: `Src/Gui/Scene/PadNav/tray_controller.cpp`
- 修改: `Src/Gui/Scene/PadNav/CMakeLists.txt`

- [ ] **步骤 1：添加 Input Monitor**

创建一个带 `Q_OBJECT` 的 `QWidget` subclass（子类）。构造函数接收 `pn::application::PadNavController&`。使用 `100 ms` 间隔的 `QTimer` 刷新以下 labels：

```text
Connected
Mapping enabled
Active layer
Left stick x/y
Right stick x/y
Button bit mask
```

Ownership（所有权）：labels 和 timer 使用 Qt parent-child ownership（父子所有权），monitor widget 作为 parent。Thread model（线程模型）：timer 运行在 GUI thread（界面线程），并且只调用 `monitorSnapshot()`。

- [ ] **步骤 2：添加 Settings Window**

创建一个带 `Q_OBJECT` 的 `QWidget` subclass。构造函数接收：

```cpp
pn::application::PadNavController&
JsonProfileRepository&
InputMonitorWindow&
```

使用：

```text
QSpinBox dead zone: 0..40, suffix "%"
QSpinBox mouse speed: 300..2400, suffix " px/s"
QSpinBox scroll speed: 120..1440, suffix " wheel units/s"
QCheckBox per ChromeActionId
QPushButton "Apply"
QPushButton "Cancel"
QPushButton "Open Input Monitor"
```

Apply 行为：

1. 构造一个标准 C++ `ChromeProfile`。
2. 使用 `pn::core::isValid` 校验。
3. 调用 `PadNavController::applyProfile`。
4. 调用 `JsonProfileRepository::save`。

Cancel 行为：从 `PadNavController::profile()` 重新加载控件状态，并隐藏窗口。

Override `closeEvent(QCloseEvent*)`，隐藏窗口并 ignore close event（忽略关闭事件）。

- [ ] **步骤 3：添加 tray controller**

创建一个带 `Q_OBJECT` 的 `QObject` subclass。构造函数接收：

```cpp
pn::application::PadNavController&
SettingsWindow&
QApplication&
```

使用 parent-child ownership 构建 `QSystemTrayIcon` 和 `QMenu`。只添加：

```text
Enable mapping / Pause mapping
Open settings
Exit
```

触发 mapping action 时，切换 controller 状态并更新 action text。退出时，调用 `PadNavController::stop()` 和 `QApplication::quit()`。

- [ ] **步骤 4：注册并构建 UI**

将全部六个 UI 文件加入 `Src/Gui/Scene/PadNav/CMakeLists.txt`。

运行：

```powershell
cmake --build cmake-build-debug --target pn_pad_nav_gui
```

预期: `pn_pad_nav_gui` 构建成功，并且 Qt MOC generation（元对象代码生成）成功。

- [ ] **步骤 5：提交 minimal UI**

```powershell
git add Src/Gui/Scene/PadNav
git commit -m "feat: add tray settings and input monitor UI"
```

### 任务 8：添加 Crash Dumps 并组装 Executable

**文件：**
- 新建: `Src/Core/Interface/WindowsPlatform/crash_dump.h`
- 新建: `Src/Core/Interface/WindowsPlatform/crash_dump.cpp`
- 修改: `Src/Core/Interface/WindowsPlatform/CMakeLists.txt`
- 修改: `Src/App/CMakeLists.txt`
- 修改: `Src/App/main.cpp`

- [ ] **步骤 1：实现 crash dump bootstrap**

Create:

```cpp
namespace pn::platform::windows {

class CrashDump final {
public:
    static void install();
    static void pruneOldDumps(std::size_t keepCount = 3U);
};

}  // namespace pn::platform::windows
```

在 `crash_dump.cpp` 中：

- 使用 `SHGetKnownFolderPath(FOLDERID_LocalAppData, ...)` 解析 `%LOCALAPPDATA%/PadNav/CrashDumps/`。
- 使用 `std::filesystem::create_directories` 创建目录。
- 安装 `SetUnhandledExceptionFilter`。
- 使用 `MiniDumpWriteDump(..., MiniDumpNormal, ...)` 写入带时间戳的 `.dmp` 文件。
- 按 `last_write_time` 降序排序 dump files，并删除最新三个之后的文件。

- [ ] **步骤 2：链接 crash dump dependencies**

将 `crash_dump.cpp` 加入 `pn_windows_platform` 并链接：

```cmake
target_link_libraries(pn_windows_platform PUBLIC
    Pn::PadNavCore
    XInput
    DbgHelp
    Shell32
)
```

- [ ] **步骤 3：组合 application objects**

按以下顺序替换 `Src/App/main.cpp`：

```cpp
QApplication app(argc, argv);
QApplication::setQuitOnLastWindowClosed(false);
QCoreApplication::setOrganizationName("PadNav");
QCoreApplication::setApplicationName("PadNav");

pn::platform::windows::CrashDump::pruneOldDumps(3U);
pn::platform::windows::CrashDump::install();

pn::application::PadNavController controller;
pn::gui::JsonProfileRepository repository;
controller.applyProfile(repository.load());
// 调试阶段可由 QTimer 调用 controller.pollOnce();
// 收敛到后台线程后再启用 controller.start();
controller.start();

pn::gui::InputMonitorWindow monitor(controller);
pn::gui::SettingsWindow settings(controller, repository, monitor);
pn::gui::TrayController tray(controller, settings, app);

return app.exec();
```

启动时不要显示 settings 或 monitor。

在 `_DEBUG` 下，将应用启动和关闭消息写入 `std::clog`。不要创建 Release Build log file（发布构建日志文件）。

- [ ] **步骤 4：链接 executable**

更新 `Src/App/CMakeLists.txt`，让 `padnav` 链接：

```cmake
target_link_libraries(padnav PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Pn::PadNavGui
    Pn::Application
    Pn::WindowsPlatform
)
```

运行：

```powershell
cmake --build cmake-build-debug --target padnav
```

预期: `padnav` 构建成功。

- [ ] **步骤 5：运行 startup smoke test（启动冒烟测试）**

从生成的 Debug output directory（调试输出目录）运行 `padnav.exe`。

预期:

```text
PadNav starts without a visible window.
The system tray contains the PadNav icon.
Open settings shows the minimal settings window.
Exit removes the tray icon and terminates the process.
```

- [ ] **步骤 6：提交 executable composition（可执行程序组装）**

```powershell
git add Src/App Src/Core/Interface/WindowsPlatform
git commit -m "feat: compose PadNav tray application"
```

### 任务 9：添加 Windows 11 手工验收 Checklist 并运行验证

**文件：**
- 新建: `Doc/Verification/PadNav-MVP-Windows-Manual-Checklist.md`

- [ ] **步骤 1：添加 manual checklist**

创建包含以下 sections（章节）的 checklist：

```text
Environment
- Windows 11
- Chrome
- Controller (BEITONG A1S2 XINPUT GAMEPAD)

Chrome 15-minute session
- Pointer movement has no obvious stutter, drift, or accidental movement
- Scroll direction and continuity are correct
- Left and right clicks work
- Base, Navigation, and Tab layer shortcuts work

Tray behavior
- Startup shows tray only
- Settings close hides the window
- Pause blocks injection while monitor still updates
- Resume restores injection
- View button toggles pause and resume once per press
- Exit terminates the process

Controller reconnect
- Disconnect keeps app alive
- Reconnect restores input automatically

Configuration
- Apply writes %APPDATA%/PadNav/config.json
- Restart restores saved values
- Cancel discards unapplied changes
- Missing or malformed JSON silently restores defaults

Input Monitor
- Shows connection, sticks, buttons, layer, enabled state
- Closing monitor does not stop mappings

Crash Dumps
- Forced unhandled exception writes MiniDumpNormal under %LOCALAPPDATA%/PadNav/CrashDumps/
- Startup leaves at most three dumps
```

- [ ] **步骤 2：运行自动化验证**

运行：

```powershell
cmake -S . -B cmake-build-debug
cmake --build cmake-build-debug --target padnav_tests padnav
ctest --test-dir cmake-build-debug -R padnav_tests --output-on-failure
```

预期: configure 成功，两个 targets 都构建成功，Core tests 通过。

- [ ] **步骤 3：运行 manual checklist**

在 Windows 11 上连接 BEITONG controller，执行每一项 checklist。记录结果为：

```text
[x] PASS
[ ] FAIL: <observed behavior>
```

- [ ] **步骤 4：提交 verification document**

```powershell
git add Doc/Verification
git commit -m "docs: add PadNav Windows acceptance checklist"
```

## 5. Plan Self-Review（计划自检）

### Spec Coverage（规格覆盖）

| Requirement（需求） | Covered By（覆盖任务） |
| --- | --- |
| XInput controller 和 reconnect（重连） | Tasks 4, 5, 9 |
| SendInput mouse、wheel 和 Chrome shortcuts | Tasks 3, 4, 9 |
| 固定 Chrome profile 和两个 modifier layers | Tasks 2, 3 |
| `125 Hz` 轮询 | Task 5 |
| 手动全局启停 | Tasks 3, 5, 7 |
| Qt-free Core 和 C++20 使用 | Tasks 2, 3, 5 |
| 使用 Qt 的 JSON 配置 | Task 6 |
| tray-only startup（仅托盘启动）和最小 UI | Tasks 7, 8 |
| 按需打开 Input Monitor | Task 7 |
| Debug-only console logging boundary（仅调试构建控制台日志边界） | 仅在 `_DEBUG` 后使用 `qDebug` 或 `std::clog`；不要添加 release file logging |
| MiniDumpNormal 和最近三个 dumps | Tasks 8, 9 |
| Core 自动化测试 | Tasks 1, 2, 3, 9 |
| Windows 11 手工验收 | Task 9 |

### Type Consistency（类型一致性）

- Core snapshot type 是 `pn::core::ControllerSnapshot`。
- Engine result type 是 `pn::core::MappingResult`。
- UI-facing snapshot type（面向 UI 的快照类型）是 `pn::application::MonitorSnapshot`。
- 持久化配置类型是 `pn::core::ChromeProfile`。
- 只有 `Gui` 使用 Qt JSON 和 Widgets。
- 只有 `WindowsPlatform` 使用 Win32 structures 和 functions。

## 6. 执行注意事项

- 保留无关 worktree changes（工作区改动）。每个任务只 stage 自己触碰的文件。
- 每个新增文本文件都保持 `UTF-8 BOM` 编码和 `CRLF` 换行。
- 优先使用职责单一的小 source files。
- 明确使用 `PRIVATE`、`PUBLIC` 和 `INTERFACE` target scopes。
- 不引入全局 `include_directories`。
- 只在非显然生命周期、线程或平台决策处添加注释。

