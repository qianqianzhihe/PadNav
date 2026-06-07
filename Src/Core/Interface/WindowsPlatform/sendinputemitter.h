#ifndef PADNAV_SENDINPUTEMITTER_H
#define PADNAV_SENDINPUTEMITTER_H

#include "PadNav/action.h"

namespace pn::platform {

class SendInputEmitter final {
public:
    [[nodiscard]] auto emit(const pn::core::Action& action) const -> bool;
};

} // namespace pn::platform

#endif // PADNAV_SENDINPUTEMITTER_H
