#ifndef PADNAV_JSONPROFILEREPOSITORY_H
#define PADNAV_JSONPROFILEREPOSITORY_H

#include "PadNav/profile.h"

#include <QString>

namespace pn::gui {

class JsonProfileRepository final {
public:
    [[nodiscard]] auto load() const -> pn::core::ChromeProfile;
    [[nodiscard]] auto save(const pn::core::ChromeProfile& profile) const -> bool;

private:
    [[nodiscard]] auto filePath() const -> QString;
};

} // namespace pn::gui

#endif // PADNAV_JSONPROFILEREPOSITORY_H
