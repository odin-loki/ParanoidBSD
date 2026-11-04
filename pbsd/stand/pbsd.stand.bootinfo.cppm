module;
#include <cstdint>

export module pbsd.stand.bootinfo;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/i386/common/bootargs.h, stand/*/bootinfo*.c, sys/reboot.h
export namespace pbsd::stand::bootinfo {

inline constexpr unsigned kBootinfoVersion = 1;
inline constexpr unsigned kBiSizeOffset      = 48; // offsetof(struct bootinfo, bi_size)

/// reboot.h RB_* flags passed through bootargs.howto.
enum class RebootFlag : unsigned int {
    Autoboot  = 0,
    Askname   = 0x001,
    Single    = 0x002,
    Nosync    = 0x004,
    Halt      = 0x008,
    Initname  = 0x010,
    Dfltroot  = 0x020,
    Kdb       = 0x040,
    Rdonly    = 0x080,
    Dump      = 0x100,
    Bootinfo  = 0x80000000u,
};

/// bootargs.h KARGS_FLAGS_*.
enum class KargsFlag : unsigned int {
    Cd      = 0x0001,
    Pxe     = 0x0002,
    Zfs     = 0x0004,
    Extarg  = 0x0008,
    Geli    = 0x0010,
};

struct BootArgs {
    std::uint32_t howto{};
    std::uint32_t bootdev{};
    std::uint32_t bootflags{};
    std::uint64_t zfspool{};
    std::uint32_t bootinfo_ptr{};
};

struct BootInfo {
    std::uint32_t version{};
    std::uint32_t size{};
    std::uint32_t flags{};
    std::uint32_t memsizes_valid{};
    std::uint32_t basemem_kb{};
    std::uint32_t extmem_kb{};
    std::uint64_t envp{};
    std::uint64_t modulep{};
    std::uint64_t kernend{};
    std::uint64_t symtab{};
    std::uint64_t esymtab{};
    std::uint64_t kernelname_ptr{};
};

[[nodiscard]] inline Status validate_bootinfo(BootInfo const& bi) noexcept {
    if (bi.version != kBootinfoVersion) {
        return Status::Protocol;
    }
    if (bi.size < kBiSizeOffset) {
        return Status::Invalid;
    }
    if (bi.memsizes_valid != 0 && bi.basemem_kb == 0 && bi.extmem_kb == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool has_bootinfo_flag(unsigned howto) noexcept {
    return (howto & static_cast<unsigned>(RebootFlag::Bootinfo)) != 0;
}

[[nodiscard]] inline Status validate_reboot_flags(unsigned howto) noexcept {
    if ((howto & static_cast<unsigned>(RebootFlag::Halt)) != 0
        && (howto & static_cast<unsigned>(RebootFlag::Dump)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline constexpr bool root_from_zfs(unsigned kargs_flags) noexcept {
    return (kargs_flags & static_cast<unsigned>(KargsFlag::Zfs)) != 0;
}

} // namespace pbsd::stand::bootinfo
