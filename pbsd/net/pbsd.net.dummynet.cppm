module;
#include <cstdint>

export module pbsd.net.dummynet;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netpfil/ipfw/ip_dn_private.h, dn_sched.h — dummynet.
export namespace pbsd::net::dummynet {

inline constexpr unsigned kMultiQueue = 0x01;

enum class SchedulerFlag : unsigned {
    MultiQueue = kMultiQueue,
};

struct Parms {
    unsigned id{};
    int red_lookup_depth{};
    int red_avg_pkt_size{};
    int red_max_pkt_size{};
    int hash_size{};
    int max_hash_size{};
    long byte_limit{};
    long slot_limit{};
    unsigned expire{};
    unsigned expire_cycle{};
    unsigned long io_pkt{};
    unsigned long io_pkt_drop{};
};

struct FlowsetCounters {
    int schk_count{};
    int si_count{};
    int fsk_count{};
    int queue_count{};
};

[[nodiscard]] inline Status validate_hash_size(int size, int max_size) noexcept {
    if (size <= 0 || max_size <= 0 || size > max_size) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_limits(long byte_limit, long slot_limit) noexcept {
    if (byte_limit < 0 || slot_limit < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init_parms(Parms& p) noexcept {
    if (validate_hash_size(p.hash_size, p.max_hash_size) != Status::Ok) {
        return Status::Invalid;
    }
    if (validate_limits(p.byte_limit, p.slot_limit) != Status::Ok) {
        return Status::Invalid;
    }
    ++p.id;
    return Status::Ok;
}

[[nodiscard]] inline bool supports_multiqueue(unsigned flags) noexcept {
    return (flags & kMultiQueue) != 0;
}

[[nodiscard]] inline void record_drop(Parms& p) noexcept {
    ++p.io_pkt_drop;
}

[[nodiscard]] inline void record_forward(Parms& p, bool fast_path) noexcept {
    ++p.io_pkt;
    (void)fast_path;
}

} // namespace pbsd::net::dummynet
