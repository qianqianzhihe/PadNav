#ifndef PADNAV_XINPUTCONTROLLER_H
#define PADNAV_XINPUTCONTROLLER_H

#include "PadNav/controllerstate.h"

namespace pn::platform {

class XInputController final {
public:
    [[nodiscard]] auto poll() const -> pn::core::ControllerSnapshot;
};

} // namespace pn::platform

#endif // PADNAV_XINPUTCONTROLLER_H
