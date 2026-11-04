module;

#if defined(__FreeBSD__)
#include <sys/capsicum.h>
#endif
#include <cstddef>
#include <cstdint>

export module pbsd.userland.capsicum.rights;

export import pbsd.core;
export import pbsd.rights;

/// Deeper rights tables from hbsd/src/lib/libcapsicum/capsicum_helpers.h
export namespace pbsd::userland::capsicum::rights {

enum class HelperFlag : unsigned {
    IgnoreEbadf = 0x0001,
    Read        = 0x0002,
    Write       = 0x0004,
    Lookup      = 0x0008,
};

[[nodiscard]] constexpr HelperFlag operator|(HelperFlag a, HelperFlag b) noexcept {
    return static_cast<HelperFlag>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

/// caph_stream_cmds — ioctl whitelist for stream fds.
inline constexpr unsigned long kStreamIoctlCmds[] = {
#if defined(__FreeBSD__)
#ifdef TIOCGETA
    TIOCGETA,
#endif
#ifdef TIOCGWINSZ
    TIOCGWINSZ,
#endif
#ifdef FIODTYPE
    FIODTYPE,
#endif
#endif
};

inline constexpr std::size_t kStreamIoctlCount =
    sizeof(kStreamIoctlCmds) / sizeof(kStreamIoctlCmds[0]);

#if defined(__FreeBSD__)
inline constexpr std::uint32_t kStreamFcntlRights = CAP_FCNTL_GETFL;
#else
inline constexpr std::uint32_t kStreamFcntlRights = 0;
#endif

/// Base rights applied before Read/Write/Lookup narrowing (caph_stream_rights).
struct StreamRightsTable {
    CapabilityRights base;
    CapabilityRights read;
    CapabilityRights write;
    CapabilityRights lookup;
};

[[nodiscard]] inline StreamRightsTable stream_rights_table() noexcept {
    StreamRightsTable t{};
    t.base = CapabilityRights::Read | CapabilityRights::Write;
    t.read = CapabilityRights::Read;
    t.write = CapabilityRights::Write;
    t.lookup = CapabilityRights::Map;
    return t;
}

[[nodiscard]] inline CapabilityRights rights_for_flags(HelperFlag flags) noexcept {
    const auto table = stream_rights_table();
    CapabilityRights r = table.base;
    if ((static_cast<unsigned>(flags) & static_cast<unsigned>(HelperFlag::Read)) != 0u) {
        r = r | table.read;
    }
    if ((static_cast<unsigned>(flags) & static_cast<unsigned>(HelperFlag::Write)) != 0u) {
        r = r | table.write;
    }
    if ((static_cast<unsigned>(flags) & static_cast<unsigned>(HelperFlag::Lookup)) != 0u) {
        r = r | table.lookup;
    }
    return r;
}

#if defined(__FreeBSD__)

struct CapBitEntry {
    unsigned long long cap;
    CapabilityRights right;
};

/// Selected CAP_* → pbsd CapabilityRights mapping for rights_limit().
inline constexpr CapBitEntry kCapBitTable[] = {
    {CAP_READ, CapabilityRights::Read},
    {CAP_WRITE, CapabilityRights::Write},
    {CAP_LOOKUP, CapabilityRights::Map},
    {CAP_GRANT, CapabilityRights::Grant},
    {CAP_FSTAT, CapabilityRights::Read},
    {CAP_SEEK, CapabilityRights::Read},
};

[[nodiscard]] inline CapabilityRights from_cap_rights(const cap_rights_t* rights) noexcept {
    CapabilityRights r = CapabilityRights::None;
    if (rights == nullptr) {
        return r;
    }
    for (const auto& e : kCapBitTable) {
        if (cap_rights_is_set(rights, e.cap)) {
            r = r | e.right;
        }
    }
    return r;
}

#else

[[nodiscard]] inline CapabilityRights from_cap_rights(const void* /*rights*/) noexcept {
    return CapabilityRights::None;
}

#endif

} // namespace pbsd::userland::capsicum::rights
