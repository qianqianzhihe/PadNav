#ifndef PADNAV_ACTION_H
#define PADNAV_ACTION_H

#include <variant>
#include <vector>

namespace pn::core {

enum class KeyCode {
    Left,
    Right,
    R,
    Escape,
    Home,
    End,
    Plus,
    Minus,
    Digit0,
    L,
    T,
    W,
    Tab,
    F
};

enum class Modifier {
    Alt,
    Control,
    Shift
};

enum class MouseButton {
    Left,
    Right
};

enum class ButtonTransition {
    Press,
    Release
};

struct MouseMove final {
    int deltaX{};
    int deltaY{};
};

struct MouseButtonAction final {
    MouseButton button{MouseButton::Left};
    ButtonTransition transition{ButtonTransition::Press};
};

struct Scroll final {
    int wheelUnits{};
};

struct KeyChord final {
    std::vector<Modifier> modifiers;
    KeyCode key{KeyCode::Escape};
};

using Action = std::variant<MouseMove, MouseButtonAction, Scroll, KeyChord>;

} // namespace pn::core

#endif // PADNAV_ACTION_H
