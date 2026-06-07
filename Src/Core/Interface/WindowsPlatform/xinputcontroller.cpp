#include "WindowsPlatform/xinputcontroller.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <Xinput.h>

#include <algorithm>

namespace pn::platform {

namespace {

[[nodiscard]] auto normalizedThumb(SHORT value) -> float {
    if (value < 0) {
        return std::clamp(static_cast<float>(value) / 32768.0F, -1.0F, 0.0F);
    }
    return std::clamp(static_cast<float>(value) / 32767.0F, 0.0F, 1.0F);
}

void appendButton(std::uint16_t& buttons,
                  WORD xinputButtons,
                  WORD xinputButton,
                  pn::core::ControllerButton controllerButton) {
    if ((xinputButtons & xinputButton) != 0U) {
        buttons |= pn::core::mask(controllerButton);
    }
}

[[nodiscard]] auto mapButtons(WORD xinputButtons) -> std::uint16_t {
    auto buttons = std::uint16_t{};
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_A, pn::core::ControllerButton::A);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_B, pn::core::ControllerButton::B);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_X, pn::core::ControllerButton::X);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_Y, pn::core::ControllerButton::Y);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, pn::core::ControllerButton::LeftShoulder);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, pn::core::ControllerButton::RightShoulder);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_BACK, pn::core::ControllerButton::View);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_DPAD_UP, pn::core::ControllerButton::DPadUp);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_DPAD_DOWN, pn::core::ControllerButton::DPadDown);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_DPAD_LEFT, pn::core::ControllerButton::DPadLeft);
    appendButton(buttons, xinputButtons, XINPUT_GAMEPAD_DPAD_RIGHT, pn::core::ControllerButton::DPadRight);
    return buttons;
}

} // namespace

auto XInputController::poll() const -> pn::core::ControllerSnapshot {
    auto state = XINPUT_STATE{};
    const auto result = XInputGetState(0, &state);
    if (result != ERROR_SUCCESS) {
        return pn::core::ControllerSnapshot{.connected = false};
    }

    const auto& gamepad = state.Gamepad;
    return pn::core::ControllerSnapshot{
        .connected = true,
        .buttons = mapButtons(gamepad.wButtons),
        .leftStick =
            pn::core::Stick{
                .x = normalizedThumb(gamepad.sThumbLX),
                .y = normalizedThumb(gamepad.sThumbLY),
            },
        .rightStick =
            pn::core::Stick{
                .x = normalizedThumb(gamepad.sThumbRX),
                .y = normalizedThumb(gamepad.sThumbRY),
            },
    };
}

} // namespace pn::platform
