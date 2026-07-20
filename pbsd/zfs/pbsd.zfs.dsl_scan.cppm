module;
#include <cstdint>

export module pbsd.zfs.dsl_scan;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/contrib/openzfs/uts/common/fs/zfs/dsl_scan.c — pool scan state.
export namespace pbsd::zfs::dsl_scan {

enum class State : unsigned char {
    None = 0,
    Scan = 1,
    Done = 2,
    Canceled = 3,
};

struct Progress {
    State state{State::None};
    std::uint64_t examined{};
    std::uint64_t total{};
};

[[nodiscard]] inline Status start(Progress& p, std::uint64_t total) noexcept {
    if (p.state == State::Scan || total == 0) {
        return Status::Invalid;
    }
    p.state = State::Scan;
    p.examined = 0;
    p.total = total;
    return Status::Ok;
}

[[nodiscard]] inline Status advance(Progress& p, std::uint64_t delta) noexcept {
    if (p.state != State::Scan) {
        return Status::Invalid;
    }
    p.examined += delta;
    if (p.examined >= p.total) {
        p.state = State::Done;
    }
    return Status::Ok;
}

} // namespace pbsd::zfs::dsl_scan
