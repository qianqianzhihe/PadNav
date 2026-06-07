#include "Application/padnavcontroller.h"

#include "PadNav/profile.h"

#include <stdexcept>
#include <string_view>

namespace {

void require(bool condition, std::string_view message) {
    if (!condition) {
        throw std::runtime_error{std::string{message}};
    }
}

} // namespace

void runPadNavControllerTests() {
    pn::application::PadNavController controller;

    require(controller.mappingEnabled(), "mapping starts enabled");
    const auto initialSnapshot = controller.monitorSnapshot();
    require(initialSnapshot.mappingEnabled, "snapshot starts enabled");
    require(initialSnapshot.pollCount == 0, "snapshot starts before polling");
    require(initialSnapshot.activeLayer == pn::core::ActiveLayer::Base, "snapshot starts on base layer");

    controller.pollOnce();
    const auto firstSnapshot = controller.monitorSnapshot();
    require(firstSnapshot.pollCount == 1, "pollOnce advances snapshot");
    require(firstSnapshot.mappingEnabled, "pollOnce preserves enabled state");

    controller.setMappingEnabled(false);
    require(!controller.mappingEnabled(), "mapping can be disabled");
    const auto disabledSnapshot = controller.monitorSnapshot();
    require(!disabledSnapshot.mappingEnabled, "snapshot follows disabled state");

    controller.pollOnce();
    const auto secondSnapshot = controller.monitorSnapshot();
    require(secondSnapshot.pollCount == 2, "pollOnce keeps advancing while disabled");
    require(!secondSnapshot.mappingEnabled, "disabled state survives poll");

    auto profile = pn::core::makeDefaultChromeProfile();
    profile.deadZonePercent = 20;
    controller.applyProfile(profile);
    require(controller.profile().deadZonePercent == 20, "profile can be applied");

    controller.start();
    controller.stop();
}
