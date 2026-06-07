#include "PadNav/mappingengine.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <span>

namespace pn::core {

namespace {

struct ScaledStick final {
    float x{};
    float y{};
};

struct ShortcutSpec final {
    ControllerButton button{};
    ChromeActionId actionId{};
    KeyCode key{};
    std::array<Modifier, 2> modifiers{};
    std::size_t modifierCount{};
};

constexpr auto navigationShortcuts = std::array{
    ShortcutSpec{.button = ControllerButton::A,
                 .actionId = ChromeActionId::Reload,
                 .key = KeyCode::R,
                 .modifiers = {Modifier::Control},
                 .modifierCount = 1},
    ShortcutSpec{.button = ControllerButton::B,
                 .actionId = ChromeActionId::StopLoading,
                 .key = KeyCode::Escape,
                 .modifiers = {},
                 .modifierCount = 0},
    ShortcutSpec{.button = ControllerButton::X,
                 .actionId = ChromeActionId::PageTop,
                 .key = KeyCode::Home,
                 .modifiers = {},
                 .modifierCount = 0},
    ShortcutSpec{.button = ControllerButton::Y,
                 .actionId = ChromeActionId::PageBottom,
                 .key = KeyCode::End,
                 .modifiers = {},
                 .modifierCount = 0},
    ShortcutSpec{.button = ControllerButton::DPadUp,
                 .actionId = ChromeActionId::ZoomIn,
                 .key = KeyCode::Plus,
                 .modifiers = {Modifier::Control},
                 .modifierCount = 1},
    ShortcutSpec{.button = ControllerButton::DPadDown,
                 .actionId = ChromeActionId::ZoomOut,
                 .key = KeyCode::Minus,
                 .modifiers = {Modifier::Control},
                 .modifierCount = 1},
    ShortcutSpec{.button = ControllerButton::DPadLeft,
                 .actionId = ChromeActionId::ResetZoom,
                 .key = KeyCode::Digit0,
                 .modifiers = {Modifier::Control},
                 .modifierCount = 1},
    ShortcutSpec{.button = ControllerButton::DPadRight,
                 .actionId = ChromeActionId::FocusAddressBar,
                 .key = KeyCode::L,
                 .modifiers = {Modifier::Control},
                 .modifierCount = 1},
};

constexpr auto tabShortcuts = std::array{
    ShortcutSpec{.button = ControllerButton::A,
                 .actionId = ChromeActionId::NewTab,
                 .key = KeyCode::T,
                 .modifiers = {Modifier::Control},
                 .modifierCount = 1},
    ShortcutSpec{.button = ControllerButton::B,
                 .actionId = ChromeActionId::CloseTab,
                 .key = KeyCode::W,
                 .modifiers = {Modifier::Control},
                 .modifierCount = 1},
    ShortcutSpec{.button = ControllerButton::X,
                 .actionId = ChromeActionId::PreviousTab,
                 .key = KeyCode::Tab,
                 .modifiers = {Modifier::Control, Modifier::Shift},
                 .modifierCount = 2},
    ShortcutSpec{.button = ControllerButton::Y,
                 .actionId = ChromeActionId::NextTab,
                 .key = KeyCode::Tab,
                 .modifiers = {Modifier::Control},
                 .modifierCount = 1},
    ShortcutSpec{.button = ControllerButton::DPadLeft,
                 .actionId = ChromeActionId::RestoreClosedTab,
                 .key = KeyCode::T,
                 .modifiers = {Modifier::Control, Modifier::Shift},
                 .modifierCount = 2},
    ShortcutSpec{.button = ControllerButton::DPadRight,
                 .actionId = ChromeActionId::FindInPage,
                 .key = KeyCode::F,
                 .modifiers = {Modifier::Control},
                 .modifierCount = 1},
};

constexpr auto baseShortcuts = std::array{
    ShortcutSpec{.button = ControllerButton::X,
                 .actionId = ChromeActionId::Back,
                 .key = KeyCode::Left,
                 .modifiers = {Modifier::Alt},
                 .modifierCount = 1},
    ShortcutSpec{.button = ControllerButton::Y,
                 .actionId = ChromeActionId::Forward,
                 .key = KeyCode::Right,
                 .modifiers = {Modifier::Alt},
                 .modifierCount = 1},
};

[[nodiscard]] auto selectLayer(const ControllerSnapshot& snapshot) -> ActiveLayer {
    const auto leftShoulder = isPressed(snapshot, ControllerButton::LeftShoulder);
    const auto rightShoulder = isPressed(snapshot, ControllerButton::RightShoulder);
    if (leftShoulder && rightShoulder) {
        return ActiveLayer::Ambiguous;
    }
    if (leftShoulder) {
        return ActiveLayer::Navigation;
    }
    if (rightShoulder) {
        return ActiveLayer::Tab;
    }
    return ActiveLayer::Base;
}

[[nodiscard]] auto scaleStick(Stick stick, int deadZonePercent) -> ScaledStick {
    const auto magnitude = std::sqrt((stick.x * stick.x) + (stick.y * stick.y));
    const auto threshold = static_cast<float>(deadZonePercent) / 100.0F;
    if (magnitude <= threshold || magnitude <= 0.0F) {
        return {};
    }

    const auto scaledMagnitude = std::clamp((magnitude - threshold) / (1.0F - threshold), 0.0F, 1.0F);
    return ScaledStick{
        .x = (stick.x / magnitude) * scaledMagnitude,
        .y = (stick.y / magnitude) * scaledMagnitude,
    };
}

[[nodiscard]] auto secondsFrom(std::chrono::milliseconds elapsed) -> float {
    return static_cast<float>(elapsed.count()) / 1000.0F;
}

[[nodiscard]] auto toDelta(float value) -> int {
    return static_cast<int>(std::lround(value));
}

[[nodiscard]] auto isRisingEdge(const ControllerSnapshot& previous,
                                const ControllerSnapshot& current,
                                ControllerButton button) -> bool {
    return !isPressed(previous, button) && isPressed(current, button);
}

[[nodiscard]] auto isFallingEdge(const ControllerSnapshot& previous,
                                 const ControllerSnapshot& current,
                                 ControllerButton button) -> bool {
    return isPressed(previous, button) && !isPressed(current, button);
}

void appendMouseMove(std::vector<Action>& actions,
                     const ControllerSnapshot& current,
                     const ChromeProfile& profile,
                     std::chrono::milliseconds elapsed) {
    const auto stick = scaleStick(current.rightStick, profile.deadZonePercent);
    const auto seconds = secondsFrom(elapsed);
    const auto deltaX = toDelta(stick.x * static_cast<float>(profile.mouseMaxSpeedPixelsPerSecond) * seconds);
    const auto deltaY = toDelta(-stick.y * static_cast<float>(profile.mouseMaxSpeedPixelsPerSecond) * seconds);
    if (deltaX != 0 || deltaY != 0) {
        actions.emplace_back(MouseMove{.deltaX = deltaX, .deltaY = deltaY});
    }
}

void appendScroll(std::vector<Action>& actions,
                  const ControllerSnapshot& current,
                  const ChromeProfile& profile,
                  std::chrono::milliseconds elapsed) {
    const auto stick = scaleStick(current.leftStick, profile.deadZonePercent);
    const auto seconds = secondsFrom(elapsed);
    const auto wheelUnits = toDelta(stick.y * static_cast<float>(profile.scrollSpeedWheelUnitsPerSecond) * seconds);
    if (wheelUnits != 0) {
        actions.emplace_back(Scroll{.wheelUnits = wheelUnits});
    }
}

void appendMouseButton(std::vector<Action>& actions,
                       const ControllerSnapshot& previous,
                       const ControllerSnapshot& current,
                       ControllerButton controllerButton,
                       MouseButton mouseButton) {
    if (isRisingEdge(previous, current, controllerButton)) {
        actions.emplace_back(MouseButtonAction{.button = mouseButton, .transition = ButtonTransition::Press});
    }
    if (isFallingEdge(previous, current, controllerButton)) {
        actions.emplace_back(MouseButtonAction{.button = mouseButton, .transition = ButtonTransition::Release});
    }
}

[[nodiscard]] auto shortcutTableFor(ActiveLayer layer) -> std::span<const ShortcutSpec> {
    switch (layer) {
    case ActiveLayer::Base:
        return baseShortcuts;
    case ActiveLayer::Navigation:
        return navigationShortcuts;
    case ActiveLayer::Tab:
        return tabShortcuts;
    case ActiveLayer::Ambiguous:
        return {};
    }
    return {};
}

[[nodiscard]] auto makeKeyChord(const ShortcutSpec& spec) -> KeyChord {
    auto modifiers = std::vector<Modifier>{};
    modifiers.reserve(spec.modifierCount);
    for (std::size_t index = 0; index < spec.modifierCount; ++index) {
        modifiers.push_back(spec.modifiers[index]);
    }
    return KeyChord{.modifiers = std::move(modifiers), .key = spec.key};
}

void appendShortcuts(std::vector<Action>& actions,
                     const ControllerSnapshot& previous,
                     const ControllerSnapshot& current,
                     const ChromeProfile& profile,
                     ActiveLayer layer) {
    for (const auto& shortcut : shortcutTableFor(layer)) {
        if (isRisingEdge(previous, current, shortcut.button) && isEnabled(profile, shortcut.actionId)) {
            actions.emplace_back(makeKeyChord(shortcut));
        }
    }
}

} // namespace

void MappingEngine::setProfile(ChromeProfile profile) {
    m_profile = profile;
}

void MappingEngine::setMappingEnabled(bool enabled) {
    m_mappingEnabled = enabled;
}

auto MappingEngine::map(const ControllerSnapshot& current,
                        std::chrono::milliseconds elapsed) -> MappingResult {
    auto result = MappingResult{.activeLayer = selectLayer(current)};

    if (current.connected && m_mappingEnabled) {
        appendMouseMove(result.actions, current, m_profile, elapsed);
        appendScroll(result.actions, current, m_profile, elapsed);

        if (result.activeLayer == ActiveLayer::Base) {
            appendMouseButton(result.actions, m_previous, current, ControllerButton::A, MouseButton::Left);
            appendMouseButton(result.actions, m_previous, current, ControllerButton::B, MouseButton::Right);
        }

        appendShortcuts(result.actions, m_previous, current, m_profile, result.activeLayer);
    }

    m_previous = current;
    return result;
}

} // namespace pn::core
