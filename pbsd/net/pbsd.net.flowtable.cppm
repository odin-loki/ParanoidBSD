module;
#include <cstdint>

export module pbsd.net.flowtable;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_flow.c — flow cache scaffold.
export namespace pbsd::net::flowtable {

inline constexpr unsigned kBuckets = 256;
inline constexpr unsigned kBucketMask = kBuckets - 1;

struct Flow {
    std::uint32_t saddr{};
    std::uint32_t daddr{};
    std::uint16_t sport{};
    std::uint16_t dport{};
    std::uint8_t proto{};
    bool valid{false};
};

struct Table {
    Flow slots[kBuckets]{};
    unsigned hits{};
    unsigned misses{};
};

[[nodiscard]] inline std::uint32_t hash_flow(const Flow& f) noexcept {
    return (f.saddr ^ f.daddr ^ (static_cast<std::uint32_t>(f.sport) << 16)
            ^ f.dport ^ f.proto)
           & kBucketMask;
}

[[nodiscard]] inline Status insert(Table& t, Flow f) noexcept {
    if (f.proto == 0) {
        return Status::Invalid;
    }
    f.valid = true;
    t.slots[hash_flow(f)] = f;
    return Status::Ok;
}

[[nodiscard]] inline Status lookup(Table& t, const Flow& key, Flow& out) noexcept {
    const auto& slot = t.slots[hash_flow(key)];
    if (!slot.valid || slot.saddr != key.saddr || slot.daddr != key.daddr
        || slot.sport != key.sport || slot.dport != key.dport
        || slot.proto != key.proto) {
        ++t.misses;
        return Status::NotFound;
    }
    ++t.hits;
    out = slot;
    return Status::Ok;
}

} // namespace pbsd::net::flowtable
