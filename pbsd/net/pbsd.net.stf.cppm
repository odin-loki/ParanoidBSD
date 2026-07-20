module;
#include <cstdint>

export module pbsd.net.stf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_stf.h — 6to4/STF tunnel ioctls.
export namespace pbsd::net::stf {

enum class Ioctl : int {
    Sv4Net = 1,
    Gv4Net = 2,
    Sbr    = 3,
};

struct V4Args {
    unsigned src{};
    unsigned braddr{};
    int v4_prefixlen{};
};

[[nodiscard]] inline Status validate_prefixlen(int len) noexcept {
    if (len < 0 || len > 32) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_args(V4Args const& a) noexcept {
    return validate_prefixlen(a.v4_prefixlen);
}

} // namespace pbsd::net::stf
