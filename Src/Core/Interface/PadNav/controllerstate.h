#ifndef PADNAV_CONTROLLERSTATE_H
#define PADNAV_CONTROLLERSTATE_H

#include <cstdint>

namespace pn::core {

enum class ControllerButton : std::uint16_t {
    A = 1U << 0U,
    B = 1U << 1U,
    X = 1U << 2U,
    Y = 1U << 3U,
    LeftShoulder = 1U << 4U,
    RightShoulder = 1U << 5U,
    View = 1U << 6U,
    DPadUp = 1U << 7U,
    DPadDown = 1U << 8U,
    DPadLeft = 1U << 9U,
    DPadRight = 1U << 10U
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

constexpr auto isPressed(const ControllerSnapshot& snapshot, ControllerButton button) -> bool {
    return (snapshot.buttons & mask(button)) != 0U;
}

} // namespace pn::core

#endif // PADNAV_CONTROLLERSTATE_H
