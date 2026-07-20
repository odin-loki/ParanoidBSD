module;
#include <cstdint>

export module pbsd.net.tcp_syncache;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_syncache.h — SYN cache bucket limits.
export namespace pbsd::net::tcp_syncache {

inline constexpr unsigned kHashSize = 512;
inline constexpr unsigned kBucketLimit = 30;
inline constexpr unsigned kRexmtLimit = 3;

enum class Flag : unsigned short {
    Offload = 0x0001,
    Tfo     = 0x0002,
    Signed  = 0x0004,
};

struct Bucket {
    unsigned count{};
    unsigned rexmt_total{};
};

[[nodiscard]] inline Status admit(Bucket& b) noexcept {
    if (b.count >= kBucketLimit) {
        return Status::Busy;
    }
    ++b.count;
    return Status::Ok;
}

[[nodiscard]] inline Status remove(Bucket& b) noexcept {
    if (b.count == 0) {
        return Status::Invalid;
    }
    --b.count;
    return Status::Ok;
}

[[nodiscard]] inline Status validate_rexmt(unsigned rexmt) noexcept {
    if (rexmt > kRexmtLimit) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::tcp_syncache
