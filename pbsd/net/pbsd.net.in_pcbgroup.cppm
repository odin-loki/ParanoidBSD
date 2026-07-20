module;
#include <cstdint>

export module pbsd.net.in_pcbgroup;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/in_pcbgroup.h — PCB hash bucket groups.
export namespace pbsd::net::in_pcbgroup {

inline constexpr unsigned kDefaultGroups = 16;
inline constexpr unsigned kMaxGroups = 256;

struct Group {
    unsigned count{};
    unsigned max{kDefaultGroups};
};

[[nodiscard]] inline Status validate_group_count(unsigned groups) noexcept {
    if (groups == 0 || groups > kMaxGroups) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned bucket_index(std::uint32_t hash, unsigned groups) noexcept {
    if (validate_group_count(groups) != Status::Ok) {
        return 0;
    }
    return hash % groups;
}

[[nodiscard]] inline Status admit(Group& g) noexcept {
    if (g.count >= g.max) {
        return Status::Busy;
    }
    ++g.count;
    return Status::Ok;
}

} // namespace pbsd::net::in_pcbgroup
