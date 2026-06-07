#include "PadNav/profile.h"

#include "testsupport.h"

void runProfileTests() {
    const auto profile = pn::core::makeDefaultChromeProfile();

    require(profile.deadZonePercent == 15, "default dead zone");
    require(profile.mouseMaxSpeedPixelsPerSecond == 1200, "default mouse speed");
    require(profile.scrollSpeedWheelUnitsPerSecond == 720, "default scroll speed");
    require(profile.bindings.size() == 16, "chrome action binding count");

    require(pn::core::isValid(profile), "default profile is valid");
    require(pn::core::isEnabled(profile, pn::core::ChromeActionId::Back), "back action enabled");
    require(pn::core::isEnabled(profile, pn::core::ChromeActionId::FindInPage), "find action enabled");

    auto disabled = profile;
    disabled.bindings[0].enabled = false;
    require(!pn::core::isEnabled(disabled, disabled.bindings[0].id), "disabled binding is not enabled");

    auto invalidDeadZone = profile;
    invalidDeadZone.deadZonePercent = 41;
    require(!pn::core::isValid(invalidDeadZone), "dead zone upper bound");

    auto invalidMouseSpeed = profile;
    invalidMouseSpeed.mouseMaxSpeedPixelsPerSecond = 299;
    require(!pn::core::isValid(invalidMouseSpeed), "mouse speed lower bound");

    auto invalidScrollSpeed = profile;
    invalidScrollSpeed.scrollSpeedWheelUnitsPerSecond = 1441;
    require(!pn::core::isValid(invalidScrollSpeed), "scroll speed upper bound");
}
