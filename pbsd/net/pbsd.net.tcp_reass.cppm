module;
#include <cstdint>

export module pbsd.net.tcp_reass;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_var.h — TCP reassembly queue limits.
export namespace pbsd::net::tcp_reass {

inline constexpr unsigned kMaxQueueLen = 100;
inline constexpr unsigned kMaxSegLen = 65535;

struct QueueStats {
    unsigned queued{};
    unsigned dropped{};
    unsigned merged{};
};

[[nodiscard]] inline Status validate_seg_len(unsigned len) noexcept {
    if (len == 0 || len > kMaxSegLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status admit(QueueStats& qs, unsigned len) noexcept {
    if (validate_seg_len(len) != Status::Ok) {
        return Status::Invalid;
    }
    if (qs.queued >= kMaxQueueLen) {
        ++qs.dropped;
        return Status::Busy;
    }
    ++qs.queued;
    return Status::Ok;
}

[[nodiscard]] inline Status release(QueueStats& qs) noexcept {
    if (qs.queued == 0) {
        return Status::Invalid;
    }
    --qs.queued;
    return Status::Ok;
}

} // namespace pbsd::net::tcp_reass
