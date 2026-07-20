module;
#include <cstdint>

export module pbsd.net.tcp_stats;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp_stats.c — TCP counter buckets.
export namespace pbsd::net::tcp_stats {

struct Counters {
    unsigned segments_in{};
    unsigned segments_out{};
    unsigned retrans{};
    unsigned drops{};
};

[[nodiscard]] inline Status account_in(Counters& c, unsigned segs) noexcept {
    if (segs == 0) {
        return Status::Invalid;
    }
    c.segments_in += segs;
    return Status::Ok;
}

[[nodiscard]] inline Status account_out(Counters& c, unsigned segs) noexcept {
    if (segs == 0) {
        return Status::Invalid;
    }
    c.segments_out += segs;
    return Status::Ok;
}

[[nodiscard]] inline Status account_retrans(Counters& c) noexcept {
    ++c.retrans;
    return Status::Ok;
}

} // namespace pbsd::net::tcp_stats
