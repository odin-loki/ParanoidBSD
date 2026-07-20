module;
#include <cstdint>

export module pbsd.net.mbuf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/mbuf.h — M_* mbuf flags.
export namespace pbsd::net::mbuf {

enum class Flag : unsigned int {
    Ext     = 0x00000001,
    Pkthdr  = 0x00000002,
    Eor     = 0x00000004,
    Rdonly  = 0x00000008,
    Bcast   = 0x00000010,
    Mcast   = 0x00000020,
    Promisc = 0x00000040,
    Vlantag = 0x00000080,
    Extpg   = 0x00000100,
    Nofree  = 0x00000200,
    Tstmp   = 0x00000400,
};

inline constexpr unsigned kCopyFlags =
    static_cast<unsigned>(Flag::Pkthdr) | static_cast<unsigned>(Flag::Eor)
    | static_cast<unsigned>(Flag::Rdonly) | static_cast<unsigned>(Flag::Bcast)
    | static_cast<unsigned>(Flag::Mcast) | static_cast<unsigned>(Flag::Promisc)
    | static_cast<unsigned>(Flag::Vlantag) | static_cast<unsigned>(Flag::Tstmp);

[[nodiscard]] inline bool has_flag(unsigned flags, Flag f) noexcept {
    return (flags & static_cast<unsigned>(f)) != 0;
}

[[nodiscard]] inline Status validate_flags(unsigned flags) noexcept {
    if (has_flag(flags, Flag::Ext) && has_flag(flags, Flag::Extpg)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::mbuf
