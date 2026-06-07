#include "PadNav/profile.h"

#include <algorithm>

namespace pn::core {

namespace {

constexpr auto minDeadZonePercent = 0;
constexpr auto maxDeadZonePercent = 40;
constexpr auto minMouseSpeedPixelsPerSecond = 300;
constexpr auto maxMouseSpeedPixelsPerSecond = 2400;
constexpr auto minScrollSpeedWheelUnitsPerSecond = 120;
constexpr auto maxScrollSpeedWheelUnitsPerSecond = 1440;

constexpr auto defaultBindings() -> std::array<ActionBinding, 16> {
    return {{
        {.id = ChromeActionId::Back, .enabled = true},
        {.id = ChromeActionId::Forward, .enabled = true},
        {.id = ChromeActionId::Reload, .enabled = true},
        {.id = ChromeActionId::StopLoading, .enabled = true},
        {.id = ChromeActionId::PageTop, .enabled = true},
        {.id = ChromeActionId::PageBottom, .enabled = true},
        {.id = ChromeActionId::ZoomIn, .enabled = true},
        {.id = ChromeActionId::ZoomOut, .enabled = true},
        {.id = ChromeActionId::ResetZoom, .enabled = true},
        {.id = ChromeActionId::FocusAddressBar, .enabled = true},
        {.id = ChromeActionId::NewTab, .enabled = true},
        {.id = ChromeActionId::CloseTab, .enabled = true},
        {.id = ChromeActionId::PreviousTab, .enabled = true},
        {.id = ChromeActionId::NextTab, .enabled = true},
        {.id = ChromeActionId::RestoreClosedTab, .enabled = true},
        {.id = ChromeActionId::FindInPage, .enabled = true},
    }};
}

constexpr auto isInRange(int value, int minimum, int maximum) -> bool {
    return value >= minimum && value <= maximum;
}

} // namespace

auto makeDefaultChromeProfile() -> ChromeProfile {
    return ChromeProfile{
        .deadZonePercent = 15,
        .mouseMaxSpeedPixelsPerSecond = 1200,
        .scrollSpeedWheelUnitsPerSecond = 720,
        .bindings = defaultBindings(),
    };
}

auto isValid(const ChromeProfile& profile) -> bool {
    return isInRange(profile.deadZonePercent, minDeadZonePercent, maxDeadZonePercent)
           && isInRange(profile.mouseMaxSpeedPixelsPerSecond,
                        minMouseSpeedPixelsPerSecond,
                        maxMouseSpeedPixelsPerSecond)
           && isInRange(profile.scrollSpeedWheelUnitsPerSecond,
                        minScrollSpeedWheelUnitsPerSecond,
                        maxScrollSpeedWheelUnitsPerSecond);
}

auto isEnabled(const ChromeProfile& profile, ChromeActionId id) -> bool {
    const auto binding = std::ranges::find_if(profile.bindings, [id](const ActionBinding& item) {
        return item.id == id;
    });
    return binding != profile.bindings.end() && binding->enabled;
}

} // namespace pn::core
