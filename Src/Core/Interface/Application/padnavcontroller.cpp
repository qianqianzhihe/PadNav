//
// Created by chenml on 2026/6/6.
//

#include "padnavcontroller.h"

#include "WindowsPlatform/sendinputemitter.h"
#include "WindowsPlatform/xinputcontroller.h"

#ifdef _DEBUG
#include <iostream>
#endif

#include <array>
#include <chrono>
#include <mutex>
#include <thread>

namespace pn::application {

namespace {

constexpr auto pollInterval = std::chrono::milliseconds{8};

[[nodiscard]] auto viewRisingEdge(const pn::core::ControllerSnapshot& previous,
                                  const pn::core::ControllerSnapshot& current) -> bool {
    return !pn::core::isPressed(previous, pn::core::ControllerButton::View)
           && pn::core::isPressed(current, pn::core::ControllerButton::View);
}

#ifdef _DEBUG
struct ButtonSpec final {
    pn::core::ControllerButton button{};
    const char* name = nullptr;
};

constexpr auto buttonSpecs = std::array{
    ButtonSpec{pn::core::ControllerButton::A, "A"},
    ButtonSpec{pn::core::ControllerButton::B, "B"},
    ButtonSpec{pn::core::ControllerButton::X, "X"},
    ButtonSpec{pn::core::ControllerButton::Y, "Y"},
    ButtonSpec{pn::core::ControllerButton::LeftShoulder, "LeftShoulder"},
    ButtonSpec{pn::core::ControllerButton::RightShoulder, "RightShoulder"},
    ButtonSpec{pn::core::ControllerButton::View, "View"},
    ButtonSpec{pn::core::ControllerButton::DPadUp, "DPadUp"},
    ButtonSpec{pn::core::ControllerButton::DPadDown, "DPadDown"},
    ButtonSpec{pn::core::ControllerButton::DPadLeft, "DPadLeft"},
    ButtonSpec{pn::core::ControllerButton::DPadRight, "DPadRight"},
};

[[nodiscard]] auto activeLayerText(pn::core::ActiveLayer activeLayer) -> const char* {
    switch (activeLayer) {
    case pn::core::ActiveLayer::Base:
        return "Base";
    case pn::core::ActiveLayer::Navigation:
        return "Navigation";
    case pn::core::ActiveLayer::Tab:
        return "Tab";
    case pn::core::ActiveLayer::Ambiguous:
        return "Ambiguous";
    }

    return "Unknown";
}

[[nodiscard]] auto matchesModifiers(const std::vector<pn::core::Modifier>& modifiers,
                                    std::initializer_list<pn::core::Modifier> expected) -> bool {
    if (modifiers.size() != expected.size()) {
        return false;
    }

    return std::equal(modifiers.begin(), modifiers.end(), expected.begin(), expected.end());
}

[[nodiscard]] auto shortcutFunctionName(const pn::core::KeyChord& keyChord) -> const char* {
    using pn::core::KeyCode;
    using pn::core::Modifier;

    if (matchesModifiers(keyChord.modifiers, {Modifier::Alt}) && keyChord.key == KeyCode::Left) {
        return "Back";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Alt}) && keyChord.key == KeyCode::Right) {
        return "Forward";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control}) && keyChord.key == KeyCode::R) {
        return "Reload";
    }
    if (matchesModifiers(keyChord.modifiers, {}) && keyChord.key == KeyCode::Escape) {
        return "StopLoading";
    }
    if (matchesModifiers(keyChord.modifiers, {}) && keyChord.key == KeyCode::Home) {
        return "PageTop";
    }
    if (matchesModifiers(keyChord.modifiers, {}) && keyChord.key == KeyCode::End) {
        return "PageBottom";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control}) && keyChord.key == KeyCode::Plus) {
        return "ZoomIn";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control}) && keyChord.key == KeyCode::Minus) {
        return "ZoomOut";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control}) && keyChord.key == KeyCode::Digit0) {
        return "ResetZoom";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control}) && keyChord.key == KeyCode::L) {
        return "FocusAddressBar";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control}) && keyChord.key == KeyCode::T) {
        return "NewTab";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control}) && keyChord.key == KeyCode::W) {
        return "CloseTab";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control, Modifier::Shift}) && keyChord.key == KeyCode::Tab) {
        return "PreviousTab";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control}) && keyChord.key == KeyCode::Tab) {
        return "NextTab";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control, Modifier::Shift}) && keyChord.key == KeyCode::T) {
        return "RestoreClosedTab";
    }
    if (matchesModifiers(keyChord.modifiers, {Modifier::Control}) && keyChord.key == KeyCode::F) {
        return "FindInPage";
    }

    return "UnknownShortcut";
}

void logButtonTransitions(const pn::core::ControllerSnapshot& previous,
                          const pn::core::ControllerSnapshot& current,
                          pn::core::ActiveLayer activeLayer) {
    for (const auto& buttonSpec : buttonSpecs) {
        const auto previousPressed = pn::core::isPressed(previous, buttonSpec.button);
        const auto currentPressed = pn::core::isPressed(current, buttonSpec.button);
        if (previousPressed == currentPressed) {
            continue;
        }

        std::clog << "PadNavController: button "
                  << (currentPressed ? "pressed " : "released ")
                  << buttonSpec.name
                  << " enum="
                  << static_cast<std::uint16_t>(buttonSpec.button)
                  << " layer="
                  << activeLayerText(activeLayer)
                  << '\n';
    }
}

void logShortcutActions(const std::vector<pn::core::Action>& actions) {
    for (const auto& action : actions) {
        const auto* keyChord = std::get_if<pn::core::KeyChord>(&action);
        if (!keyChord) {
            continue;
        }

        std::clog << "PadNavController: shortcut function="
                  << shortcutFunctionName(*keyChord)
                  << '\n';
    }
}
#endif

} // namespace

class PadNavController::Impl final {
public:
    pn::platform::XInputController controllerInput;
    pn::platform::SendInputEmitter inputEmitter;
    pn::core::MappingEngine mappingEngine;

    mutable std::mutex mutex;
    mutable std::mutex threadMutex;
    pn::core::ChromeProfile profile{pn::core::makeDefaultChromeProfile()};
    pn::core::ControllerSnapshot previousController{};
    MonitorSnapshot monitorSnapshot{};
    std::jthread workerThread;
};

PadNavController::PadNavController()
    : m_impl(std::make_unique<Impl>()) {
}

PadNavController::~PadNavController() {
    stop();
}

void PadNavController::pollOnce() {
    const auto controller = m_impl->controllerInput.poll();

    auto activeProfile = pn::core::ChromeProfile{};
    auto previousController = pn::core::ControllerSnapshot{};
    {
        std::scoped_lock lock{m_impl->mutex};
        activeProfile = m_impl->profile;
        previousController = m_impl->previousController;
    }

    auto enabled = m_mappingEnabled.load();
    if (viewRisingEdge(previousController, controller)) {
        enabled = !enabled;
        m_mappingEnabled.store(enabled);
    }

    m_impl->mappingEngine.setProfile(activeProfile);
    m_impl->mappingEngine.setMappingEnabled(enabled);
    const auto mappingResult = m_impl->mappingEngine.map(controller, pollInterval);

#ifdef _DEBUG
    logButtonTransitions(previousController, controller, mappingResult.activeLayer);
    logShortcutActions(mappingResult.actions);
#endif

    for (const auto& action : mappingResult.actions) {
        if (!m_impl->inputEmitter.emit(action)) {
#ifdef _DEBUG
            std::clog << "PadNavController: SendInputEmitter::emit failed\n";
#endif
        }
    }

    {
        std::scoped_lock lock{m_impl->mutex};
#ifdef _DEBUG
        if (m_impl->previousController.connected != controller.connected) {
            std::clog << "PadNavController: controller "
                      << (controller.connected ? "connected" : "disconnected") << '\n';
        }
#endif
        m_impl->previousController = controller;
        m_impl->monitorSnapshot.controller = controller;
        m_impl->monitorSnapshot.activeLayer = mappingResult.activeLayer;
        m_impl->monitorSnapshot.mappingEnabled = enabled;
        ++m_impl->monitorSnapshot.pollCount;
    }
}

void PadNavController::start() {
    std::scoped_lock lock{m_impl->threadMutex};
    if (m_impl->workerThread.joinable()) {
        return;
    }

    m_impl->workerThread = std::jthread{[this](std::stop_token stopToken) {
        while (!stopToken.stop_requested()) {
            const auto nextPollTime = std::chrono::steady_clock::now() + pollInterval;
            pollOnce();
            std::this_thread::sleep_until(nextPollTime);
        }
    }};
}

void PadNavController::stop() {
    auto workerThread = std::jthread{};
    {
        std::scoped_lock lock{m_impl->threadMutex};
        if (!m_impl->workerThread.joinable()) {
            return;
        }

        workerThread = std::move(m_impl->workerThread);
    }

    workerThread.request_stop();
    if (workerThread.joinable()) {
        workerThread.join();
    }
}

void PadNavController::setMappingEnabled(bool enabled) {
    m_mappingEnabled.store(enabled);
    std::scoped_lock lock{m_impl->mutex};
    m_impl->monitorSnapshot.mappingEnabled = enabled;
}

auto PadNavController::mappingEnabled() const -> bool {
    return m_mappingEnabled.load();
}

void PadNavController::applyProfile(const pn::core::ChromeProfile& profile) {
    std::scoped_lock lock{m_impl->mutex};
    m_impl->profile = profile;
}

auto PadNavController::profile() const -> pn::core::ChromeProfile {
    std::scoped_lock lock{m_impl->mutex};
    return m_impl->profile;
}

auto PadNavController::monitorSnapshot() const -> MonitorSnapshot {
    std::scoped_lock lock{m_impl->mutex};
    auto snapshot = m_impl->monitorSnapshot;
    snapshot.mappingEnabled = m_mappingEnabled.load();
    return snapshot;
}


} // namespace pn::application
