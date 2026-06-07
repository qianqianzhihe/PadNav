#ifndef PADNAV_PROFILE_H
#define PADNAV_PROFILE_H

#include <array>

namespace pn::core {

enum class ChromeActionId {
    Back,
    Forward,
    Reload,
    StopLoading,
    PageTop,
    PageBottom,
    ZoomIn,
    ZoomOut,
    ResetZoom,
    FocusAddressBar,
    NewTab,
    CloseTab,
    PreviousTab,
    NextTab,
    RestoreClosedTab,
    FindInPage
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

[[nodiscard]] auto makeDefaultChromeProfile() -> ChromeProfile;
[[nodiscard]] auto isValid(const ChromeProfile& profile) -> bool;
[[nodiscard]] auto isEnabled(const ChromeProfile& profile, ChromeActionId id) -> bool;

} // namespace pn::core

#endif // PADNAV_PROFILE_H
