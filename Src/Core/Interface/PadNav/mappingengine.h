#ifndef PADNAV_MAPPINGENGINE_H
#define PADNAV_MAPPINGENGINE_H

#include "PadNav/action.h"
#include "PadNav/controllerstate.h"
#include "PadNav/profile.h"

#include <chrono>
#include <vector>

namespace pn::core {

enum class ActiveLayer {
    Base,
    Navigation,
    Tab,
    Ambiguous
};

struct MappingResult final {
    ActiveLayer activeLayer{ActiveLayer::Base};
    std::vector<Action> actions;
};

class MappingEngine final {
public:
    void setProfile(ChromeProfile profile);
    void setMappingEnabled(bool enabled);
    [[nodiscard]] auto map(const ControllerSnapshot& current,
                           std::chrono::milliseconds elapsed) -> MappingResult;

private:
    ChromeProfile m_profile{makeDefaultChromeProfile()};
    ControllerSnapshot m_previous{};
    bool m_mappingEnabled{true};
};

} // namespace pn::core

#endif // PADNAV_MAPPINGENGINE_H
