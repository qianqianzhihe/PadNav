#ifndef PADNAV_CRASHDUMP_H
#define PADNAV_CRASHDUMP_H

#include <cstddef>

namespace pn::platform {

class CrashDump final {
public:
    static void install();
    static void pruneOldDumps(std::size_t keepCount = 3U);
};

} // namespace pn::platform

#endif // PADNAV_CRASHDUMP_H
