module;
#include <cstdint>

export module pbsd.fs.mount;

import pbsd.core;
import pbsd.rights;

using pbsd::CapabilityRights;
using pbsd::has_right;

/// PROVENANCE: hbsd/src/sys/sys/mount.h — MNT_* mount flags and statfs version.
export namespace pbsd::fs::mount {

inline constexpr std::uint32_t kStatfsVersion = 0x20140518u;
inline constexpr std::size_t   kMfsNameLen      = 16;
inline constexpr std::size_t   kMNameLen        = 1024;

enum class Flag : unsigned long long {
    Rdonly       = 0x0000000000000001ull, // MNT_RDONLY
    Synchronous  = 0x0000000000000002ull, // MNT_SYNCHRONOUS
    Noexec       = 0x0000000000000004ull, // MNT_NOEXEC
    Nosuid       = 0x0000000000000008ull, // MNT_NOSUID
    Nfs4Acls     = 0x0000000000000010ull, // MNT_NFS4ACLS
    Union        = 0x0000000000000020ull, // MNT_UNION
    Async        = 0x0000000000000040ull, // MNT_ASYNC
    Suj          = 0x0000000100000000ull, // MNT_SUJ
    Automounted  = 0x0000000200000000ull, // MNT_AUTOMOUNTED
    Untrusted    = 0x0000000800000000ull, // MNT_UNTRUSTED
    Noatime      = 0x0000000010000000ull, // MNT_NOATIME
    Gjournal     = 0x0000000002000000ull, // MNT_GJOURNAL
    Multilabel   = 0x0000000004000000ull, // MNT_MULTILABEL
    Acls         = 0x0000000008000000ull, // MNT_ACLS
    Exported     = 0x0000000000000100ull, // MNT_EXPORTED
    Local        = 0x0000000000001000ull, // MNT_LOCAL
    Quota        = 0x0000000000002000ull, // MNT_QUOTA
    Root         = 0x0000000000004000ull, // MNT_ROOTFS
};

struct FlagEntry {
    Flag          flag{};
    const char*   name{};
    CapabilityRights need_read{CapabilityRights::Read};
    CapabilityRights need_write{CapabilityRights::Write};
};

inline constexpr FlagEntry kMountFlagTable[] = {
    {Flag::Rdonly, "rdonly", CapabilityRights::Read, CapabilityRights::None},
    {Flag::Noexec, "noexec", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Nosuid, "nosuid", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Noatime, "noatime", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Union, "union", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Grant},
    {Flag::Gjournal, "gjournal", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Grant},
    {Flag::Multilabel, "multilabel", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Acls, "acls", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Exported, "exported", CapabilityRights::Read, CapabilityRights::Write | CapabilityRights::Grant},
    {Flag::Untrusted, "untrusted", CapabilityRights::Read, CapabilityRights::Write},
    {Flag::Automounted, "automounted", CapabilityRights::Read, CapabilityRights::None},
    {Flag::Root, "root", CapabilityRights::Read, CapabilityRights::None},
};

[[nodiscard]] inline constexpr std::size_t mount_flag_table_size() noexcept {
    return sizeof(kMountFlagTable) / sizeof(kMountFlagTable[0]);
}

[[nodiscard]] inline Status check_mount_flag(CapabilityRights rights, Flag f,
                                             bool setting) noexcept {
    for (auto const& e : kMountFlagTable) {
        if (e.flag != f) {
            continue;
        }
        if (!has_right(rights, e.need_read)) {
            return Status::Denied;
        }
        if (setting && e.need_write != CapabilityRights::None
            && !has_right(rights, e.need_write)) {
            return Status::Denied;
        }
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] inline constexpr bool flag_is_set(unsigned long long mask, Flag f) noexcept {
    return (mask & static_cast<unsigned long long>(f)) != 0;
}

} // namespace pbsd::fs::mount
