module;
#include <cstdint>

export module pbsd.geom.dump;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/geom_dump.c — topology dump buffer planner.
export namespace pbsd::geom::dump {

inline constexpr unsigned kMaxLines = 4096;

struct DumpBuf {
    unsigned lines{};
    unsigned capacity{kMaxLines};
    bool truncated{false};
};

[[nodiscard]] inline Status reset(DumpBuf& b) noexcept {
    b.lines = 0;
    b.truncated = false;
    return Status::Ok;
}

[[nodiscard]] inline Status append_line(DumpBuf& b) noexcept {
    if (b.lines >= b.capacity) {
        b.truncated = true;
        return Status::Busy;
    }
    ++b.lines;
    return Status::Ok;
}

[[nodiscard]] inline std::uint64_t bytes_estimate(const DumpBuf& b,
                                                  unsigned avg_line) noexcept {
    return static_cast<std::uint64_t>(b.lines) * avg_line;
}

} // namespace pbsd::geom::dump
