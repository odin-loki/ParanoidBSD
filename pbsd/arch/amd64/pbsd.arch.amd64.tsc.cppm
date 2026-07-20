module;
#include <cstdint>

export module pbsd.arch.amd64.tsc;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64 — TSC calibration scaffold.
export namespace pbsd::arch::amd64::tsc {

struct Calib {
    std::uint64_t hz{};
    bool ready{false};
};

[[nodiscard]] inline Status calibrate(Calib& c, std::uint64_t hz) noexcept {
    if (hz == 0) {
        return Status::Invalid;
    }
    c.hz = hz;
    c.ready = true;
    return Status::Ok;
}

[[nodiscard]] inline Status validate(const Calib& c) noexcept {
    return c.ready && c.hz != 0 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::arch::amd64::tsc
