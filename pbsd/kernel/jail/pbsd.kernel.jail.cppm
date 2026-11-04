module;
#include <cstdint>

export module pbsd.kernel.jail;

import pbsd.core;

/// Freestanding port of `sys/jail.h` kernel-side prison states, flags, limits.
export namespace pbsd::kernel::jail {

inline constexpr unsigned kJailApiVersion = 2u;
inline constexpr unsigned kXprisonVersion = 3u;
inline constexpr int      kJailMax        = 999999;

inline constexpr unsigned kJailCreate   = 0x01u;
inline constexpr unsigned kJailUpdate   = 0x02u;
inline constexpr unsigned kJailAttach   = 0x04u;
inline constexpr unsigned kJailDying     = 0x08u;
inline constexpr unsigned kJailUseDesc  = 0x10u;
inline constexpr unsigned kJailAtDesc   = 0x20u;
inline constexpr unsigned kJailGetDesc  = 0x40u;
inline constexpr unsigned kJailOwnDesc  = 0x80u;
inline constexpr unsigned kJailSetMask  = 0xffu;
inline constexpr unsigned kJailGetMask  = 0xf8u;

inline constexpr int kJailSysDisable  = 0;
inline constexpr int kJailSysNew      = 1;
inline constexpr int kJailSysInherit  = 2;

enum class PrisonState : int {
    Invalid = 0,
    Alive   = 1,
    Dying   = 2,
};

enum class PrisonFamily : int {
    Inet   = 0,
    Inet6  = 1,
    Max    = 2,
};

struct PrisonId {
    int         id{0};
    PrisonState state{PrisonState::Invalid};
    unsigned    path_hash{0};
};

[[nodiscard]] constexpr bool is_alive(PrisonState s) noexcept {
    return s == PrisonState::Alive;
}

[[nodiscard]] constexpr Status validate_jail_id(int jid) noexcept {
    if (jid < 0 || jid > kJailMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_jail_flags(unsigned flags, bool setting) noexcept {
    const unsigned mask = setting ? kJailSetMask : kJailGetMask;
    if ((flags & ~mask) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool flag_create(unsigned f) noexcept {
    return (f & kJailCreate) != 0;
}
[[nodiscard]] constexpr bool flag_update(unsigned f) noexcept {
    return (f & kJailUpdate) != 0;
}
[[nodiscard]] constexpr bool flag_attach(unsigned f) noexcept {
    return (f & kJailAttach) != 0;
}

struct JailFlagEntry {
    unsigned    flag;
    const char* name;
};

inline constexpr JailFlagEntry kFlagTable[] = {
    {kJailCreate,  "create"},
    {kJailUpdate,  "update"},
    {kJailAttach,  "attach"},
    {kJailDying,   "dying"},
    {kJailUseDesc, "use_desc"},
};

[[nodiscard]] inline unsigned flag_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kFlagTable) / sizeof(kFlagTable[0]));
}

} // namespace pbsd::kernel::jail
