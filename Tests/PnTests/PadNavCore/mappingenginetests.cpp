#include "PadNav/mappingengine.h"

#include "testsupport.h"

#include <algorithm>
#include <chrono>
#include <initializer_list>
#include <vector>

namespace {

using pn::core::Action;
using pn::core::ActiveLayer;
using pn::core::ButtonTransition;
using pn::core::ChromeActionId;
using pn::core::ControllerButton;
using pn::core::ControllerSnapshot;
using pn::core::KeyChord;
using pn::core::KeyCode;
using pn::core::MappingEngine;
using pn::core::Modifier;
using pn::core::MouseButton;
using pn::core::MouseButtonAction;
using pn::core::MouseMove;
using pn::core::Scroll;

constexpr auto sampleElapsed = std::chrono::milliseconds{8};

[[nodiscard]] auto snapshotWith(std::initializer_list<ControllerButton> buttons) -> ControllerSnapshot {
    auto snapshot = ControllerSnapshot{.connected = true};
    for (const auto button : buttons) {
        snapshot.buttons |= pn::core::mask(button);
    }
    return snapshot;
}

[[nodiscard]] auto containsMouseButton(const std::vector<Action>& actions,
                                       MouseButton button,
                                       ButtonTransition transition) -> bool {
    return std::ranges::any_of(actions, [button, transition](const Action& action) {
        const auto* mouseButton = std::get_if<MouseButtonAction>(&action);
        return mouseButton != nullptr && mouseButton->button == button && mouseButton->transition == transition;
    });
}

[[nodiscard]] auto containsKeyChord(const std::vector<Action>& actions,
                                    std::initializer_list<Modifier> modifiers,
                                    KeyCode key) -> bool {
    const auto expectedModifiers = std::vector<Modifier>{modifiers};
    return std::ranges::any_of(actions, [&expectedModifiers, key](const Action& action) {
        const auto* keyChord = std::get_if<KeyChord>(&action);
        return keyChord != nullptr && keyChord->key == key && keyChord->modifiers == expectedModifiers;
    });
}

template <typename T>
[[nodiscard]] auto containsAction(const std::vector<Action>& actions) -> bool {
    return std::ranges::any_of(actions, [](const Action& action) {
        return std::holds_alternative<T>(action);
    });
}

[[nodiscard]] auto makeEngine() -> MappingEngine {
    auto engine = MappingEngine{};
    engine.setProfile(pn::core::makeDefaultChromeProfile());
    engine.setMappingEnabled(true);
    return engine;
}

} // namespace

void runMappingEngineTests() {
    {
        auto engine = makeEngine();
        const auto result = engine.map(snapshotWith({ControllerButton::A}), sampleElapsed);
        require(result.activeLayer == ActiveLayer::Base, "base layer selected without shoulders");
        require(containsMouseButton(result.actions, MouseButton::Left, ButtonTransition::Press), "A press maps to left mouse press");
    }

    {
        auto engine = makeEngine();
        (void)engine.map(snapshotWith({ControllerButton::A}), sampleElapsed);
        const auto result = engine.map(snapshotWith({}), sampleElapsed);
        require(containsMouseButton(result.actions, MouseButton::Left, ButtonTransition::Release),
                "A release maps to left mouse release");
    }

    {
        auto engine = makeEngine();
        const auto result = engine.map(snapshotWith({ControllerButton::X}), sampleElapsed);
        require(containsKeyChord(result.actions, {Modifier::Alt}, KeyCode::Left), "X rising edge maps to Alt+Left");

        const auto held = engine.map(snapshotWith({ControllerButton::X}), sampleElapsed);
        require(!containsAction<KeyChord>(held.actions), "held X does not repeat key chord");
    }

    {
        auto engine = makeEngine();
        const auto result = engine.map(snapshotWith({ControllerButton::LeftShoulder, ControllerButton::A}), sampleElapsed);
        require(result.activeLayer == ActiveLayer::Navigation, "LB selects navigation layer");
        require(containsKeyChord(result.actions, {Modifier::Control}, KeyCode::R), "LB+A maps to Ctrl+R");
    }

    {
        auto engine = makeEngine();
        const auto result = engine.map(snapshotWith({ControllerButton::RightShoulder, ControllerButton::A}), sampleElapsed);
        require(result.activeLayer == ActiveLayer::Tab, "RB selects tab layer");
        require(containsKeyChord(result.actions, {Modifier::Control}, KeyCode::T), "RB+A maps to Ctrl+T");
    }

    {
        auto engine = makeEngine();
        const auto result = engine.map(
            snapshotWith({ControllerButton::LeftShoulder, ControllerButton::RightShoulder, ControllerButton::A}),
            sampleElapsed);
        require(result.activeLayer == ActiveLayer::Ambiguous, "LB+RB selects ambiguous layer");
        require(!containsAction<KeyChord>(result.actions), "ambiguous layer emits no layer key chord");
    }

    {
        auto engine = makeEngine();
        auto snapshot = snapshotWith({});
        snapshot.rightStick = {.x = 0.10F, .y = 0.0F};
        const auto result = engine.map(snapshot, sampleElapsed);
        require(!containsAction<MouseMove>(result.actions), "right stick inside dead zone emits no mouse move");
    }

    {
        auto engine = makeEngine();
        auto snapshot = snapshotWith({});
        snapshot.rightStick = {.x = 0.50F, .y = 0.0F};
        const auto result = engine.map(snapshot, sampleElapsed);
        require(containsAction<MouseMove>(result.actions), "right stick outside dead zone emits mouse move");
    }

    {
        auto engine = makeEngine();
        auto snapshot = snapshotWith({});
        snapshot.leftStick = {.x = 0.0F, .y = 0.60F};
        const auto result = engine.map(snapshot, sampleElapsed);
        require(containsAction<Scroll>(result.actions), "left stick outside dead zone emits scroll");
    }

    {
        auto engine = makeEngine();
        engine.setMappingEnabled(false);
        auto snapshot = snapshotWith({ControllerButton::A});
        snapshot.rightStick = {.x = 1.0F, .y = 0.0F};
        const auto result = engine.map(snapshot, sampleElapsed);
        require(result.actions.empty(), "disabled mapping emits no output actions");
    }

    {
        auto profile = pn::core::makeDefaultChromeProfile();
        for (auto& binding : profile.bindings) {
            if (binding.id == ChromeActionId::Reload) {
                binding.enabled = false;
            }
        }

        auto engine = MappingEngine{};
        engine.setProfile(profile);
        const auto result = engine.map(snapshotWith({ControllerButton::LeftShoulder, ControllerButton::A}), sampleElapsed);
        require(!containsAction<KeyChord>(result.actions), "disabled reload binding emits no key chord");
    }
}
