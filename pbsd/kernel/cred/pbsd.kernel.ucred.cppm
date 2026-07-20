module;
#include <cstdint>

export module pbsd.kernel.ucred;

import pbsd.core;

/// Freestanding port of `sys/ucred.h` helpers — flags, group membership, cred compare.
export namespace pbsd::kernel::ucred {

inline constexpr unsigned kCredFlagCapmode  = 0x00000001u;
inline constexpr unsigned kCredFlagGroupset = 0x00000002u;
inline constexpr unsigned kCredSmallgroupsNb = 16u;
inline constexpr unsigned kXuNgroups         = 16u;
inline constexpr unsigned kXucredVersion    = 0u;

struct Ucred {
    unsigned flags{0};
    unsigned ruid{0};
    unsigned rgid{0};
    unsigned euid{0};
    unsigned egid{0};
    unsigned svuid{0};
    unsigned svgid{0};
    unsigned ngroups{0};
    unsigned groups[kCredSmallgroupsNb]{};
    unsigned jail_id{0};
};

[[nodiscard]] constexpr bool in_cap_mode(unsigned flags) noexcept {
    return (flags & kCredFlagCapmode) != 0;
}

[[nodiscard]] constexpr bool groups_are_set(unsigned flags) noexcept {
    return (flags & kCredFlagGroupset) != 0;
}

[[nodiscard]] inline bool groupmember(unsigned gid, const Ucred& cred) noexcept {
    if (gid == cred.rgid || gid == cred.egid) {
        return true;
    }
    const unsigned n = cred.ngroups < kCredSmallgroupsNb ? cred.ngroups : kCredSmallgroupsNb;
    for (unsigned i = 0; i < n; ++i) {
        if (cred.groups[i] == gid) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline bool cred_equal_ids(const Ucred& a, const Ucred& b) noexcept {
    return a.ruid == b.ruid && a.rgid == b.rgid && a.euid == b.euid && a.egid == b.egid
        && a.jail_id == b.jail_id;
}

/// `crhold` refcount gate — reject overflow.
[[nodiscard]] constexpr Status validate_refcount(long ref) noexcept {
    if (ref < 0 || ref > 0x7fffffffL) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_ngroups(int ngroups) noexcept {
    if (ngroups < 0 || ngroups > static_cast<int>(kCredSmallgroupsNb)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

struct UcredFlagEntry {
    unsigned    flag;
    const char* name;
};

inline constexpr UcredFlagEntry kFlagTable[] = {
    {kCredFlagCapmode,  "capmode"},
    {kCredFlagGroupset, "groupset"},
};

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kFlagTable) / sizeof(kFlagTable[0]));
}

} // namespace pbsd::kernel::ucred
