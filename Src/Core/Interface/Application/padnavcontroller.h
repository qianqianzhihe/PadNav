//
// Created by chenml on 2026/6/6.
//

#ifndef PADNAV_PADNAVCONTROLLER_H
#define PADNAV_PADNAVCONTROLLER_H

#include "PadNav/controllerstate.h"
#include "PadNav/mappingengine.h"
#include "PadNav/profile.h"

#include <atomic>
#include <memory>

namespace pn::application {

struct MonitorSnapshot final {
    pn::core::ControllerSnapshot controller{};
    pn::core::ActiveLayer activeLayer{pn::core::ActiveLayer::Base};
    bool mappingEnabled{true};
    int pollCount{};
};

class PadNavController final {
public:
    PadNavController();
    ~PadNavController();

    PadNavController(const PadNavController&) = delete;
    auto operator=(const PadNavController&) -> PadNavController& = delete;

    void pollOnce();
    void start();
    void stop();
    void setMappingEnabled(bool enabled);

    [[nodiscard]] auto mappingEnabled() const -> bool;
    void applyProfile(const pn::core::ChromeProfile& profile);
    [[nodiscard]] auto profile() const -> pn::core::ChromeProfile;
    [[nodiscard]] auto monitorSnapshot() const -> MonitorSnapshot;

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
    std::atomic_bool m_mappingEnabled{true};
};

}


#endif // PADNAV_PADNAVCONTROLLER_H
