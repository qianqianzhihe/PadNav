#include "WindowsPlatform/sendinputemitter.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <algorithm>
#include <array>
#include <vector>

namespace pn::platform {

namespace {

[[nodiscard]] auto mouseInput(DWORD flags, LONG deltaX = 0, LONG deltaY = 0, DWORD mouseData = 0) -> INPUT {
    auto input = INPUT{};
    input.type = INPUT_MOUSE;
    input.mi.dx = deltaX;
    input.mi.dy = deltaY;
    input.mi.mouseData = mouseData;
    input.mi.dwFlags = flags;
    return input;
}

[[nodiscard]] auto keyInput(WORD virtualKey, bool keyUp) -> INPUT {
    auto input = INPUT{};
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = virtualKey;
    input.ki.dwFlags = keyUp ? KEYEVENTF_KEYUP : 0U;
    return input;
}

[[nodiscard]] auto sendInputs(std::vector<INPUT>& inputs) -> bool {
    if (inputs.empty()) {
        return true;
    }

    const auto sent = SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
    return sent == inputs.size();
}

[[nodiscard]] auto sendInput(INPUT input) -> bool {
    auto inputs = std::vector<INPUT>{input};
    return sendInputs(inputs);
}

[[nodiscard]] auto mouseButtonFlags(const pn::core::MouseButtonAction& action) -> DWORD {
    if (action.button == pn::core::MouseButton::Left) {
        return action.transition == pn::core::ButtonTransition::Press ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
    }
    return action.transition == pn::core::ButtonTransition::Press ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
}

[[nodiscard]] auto virtualKey(pn::core::Modifier modifier) -> WORD {
    switch (modifier) {
    case pn::core::Modifier::Alt:
        return VK_MENU;
    case pn::core::Modifier::Control:
        return VK_CONTROL;
    case pn::core::Modifier::Shift:
        return VK_SHIFT;
    }
    return 0;
}

[[nodiscard]] auto virtualKey(pn::core::KeyCode key) -> WORD {
    switch (key) {
    case pn::core::KeyCode::Left:
        return VK_LEFT;
    case pn::core::KeyCode::Right:
        return VK_RIGHT;
    case pn::core::KeyCode::R:
        return 'R';
    case pn::core::KeyCode::Escape:
        return VK_ESCAPE;
    case pn::core::KeyCode::Home:
        return VK_HOME;
    case pn::core::KeyCode::End:
        return VK_END;
    case pn::core::KeyCode::Plus:
        return VK_OEM_PLUS;
    case pn::core::KeyCode::Minus:
        return VK_OEM_MINUS;
    case pn::core::KeyCode::Digit0:
        return '0';
    case pn::core::KeyCode::L:
        return 'L';
    case pn::core::KeyCode::T:
        return 'T';
    case pn::core::KeyCode::W:
        return 'W';
    case pn::core::KeyCode::Tab:
        return VK_TAB;
    case pn::core::KeyCode::F:
        return 'F';
    }
    return 0;
}

[[nodiscard]] auto emitMouseMove(const pn::core::MouseMove& action) -> bool {
    return sendInput(mouseInput(MOUSEEVENTF_MOVE, action.deltaX, action.deltaY));
}

[[nodiscard]] auto emitScroll(const pn::core::Scroll& action) -> bool {
    return sendInput(mouseInput(MOUSEEVENTF_WHEEL, 0, 0, static_cast<DWORD>(action.wheelUnits)));
}

[[nodiscard]] auto emitMouseButton(const pn::core::MouseButtonAction& action) -> bool {
    return sendInput(mouseInput(mouseButtonFlags(action)));
}

[[nodiscard]] auto emitKeyChord(const pn::core::KeyChord& action) -> bool {
    const auto mainKey = virtualKey(action.key);
    if (mainKey == 0) {
        return false;
    }

    auto inputs = std::vector<INPUT>{};
    inputs.reserve((action.modifiers.size() * 2U) + 2U);

    for (const auto modifier : action.modifiers) {
        const auto modifierKey = virtualKey(modifier);
        if (modifierKey == 0) {
            return false;
        }
        inputs.push_back(keyInput(modifierKey, false));
    }

    inputs.push_back(keyInput(mainKey, false));
    inputs.push_back(keyInput(mainKey, true));

    for (auto iterator = action.modifiers.rbegin(); iterator != action.modifiers.rend(); ++iterator) {
        inputs.push_back(keyInput(virtualKey(*iterator), true));
    }

    return sendInputs(inputs);
}

struct EmitVisitor final {
    [[nodiscard]] auto operator()(const pn::core::MouseMove& action) const -> bool {
        return emitMouseMove(action);
    }

    [[nodiscard]] auto operator()(const pn::core::Scroll& action) const -> bool {
        return emitScroll(action);
    }

    [[nodiscard]] auto operator()(const pn::core::MouseButtonAction& action) const -> bool {
        return emitMouseButton(action);
    }

    [[nodiscard]] auto operator()(const pn::core::KeyChord& action) const -> bool {
        return emitKeyChord(action);
    }
};

} // namespace

auto SendInputEmitter::emit(const pn::core::Action& action) const -> bool {
    return std::visit(EmitVisitor{}, action);
}

} // namespace pn::platform
