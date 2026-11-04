module;
#include <cstdint>

export module pbsd.net.ip_reass;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_var.h — IPv4 fragment reassembly limits.
export namespace pbsd::net::ip_reass {

inline constexpr unsigned kMaxQueueLen = 100;
inline constexpr unsigned kMaxFragLen = 65535;
inline constexpr unsigned kMaxLifetimeSec = 30;

struct QueueStats {
    unsigned queued{};
    unsigned timed_out{};
};

[[nodiscard]] inline Status validate_frag(unsigned offset, unsigned len) noexcept {
    if (len == 0 || len > kMaxFragLen) {
        return Status::Invalid;
    }
    if ((offset & 0x7) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status admit(QueueStats& qs) noexcept {
    if (qs.queued >= kMaxQueueLen) {
        return Status::Busy;
    }
    ++qs.queued;
    return Status::Ok;
}

} // namespace pbsd::net::ip_reass
