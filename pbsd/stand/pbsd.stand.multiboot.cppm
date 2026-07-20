module;
#include <cstdint>

export module pbsd.stand.multiboot;

import pbsd.core;

/// PROVENANCE: hbsd/src/stand/i386/libi386/multiboot.c, multiboot.h — Multiboot v1 flags.
export namespace pbsd::stand::multiboot {

inline constexpr unsigned kHeaderMagic = 0x1BADB002u;
inline constexpr unsigned kBootloaderMagic = 0x2BADB002u;
inline constexpr unsigned kSearchLimit = 8192;
inline constexpr unsigned kModAlign = 0x1000;

enum class HeaderFlag : unsigned int {
    PageAlign = 0x00000001,
    MemoryInfo = 0x00000002,
    VideoMode = 0x00000004,
    AoutKludge = 0x00010000,
};

enum class InfoFlag : unsigned int {
    Memory = 0x00000001,
    BootDev = 0x00000002,
    Cmdline = 0x00000004,
    Mods = 0x00000008,
    ElfShdr = 0x00000020,
    MemMap = 0x00000040,
    BootLoaderName = 0x00000200,
};

[[nodiscard]] inline Status validate_header_magic(unsigned magic) noexcept {
    return magic == kHeaderMagic ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline unsigned supported_header_flags() noexcept {
    return static_cast<unsigned>(HeaderFlag::PageAlign)
         | static_cast<unsigned>(HeaderFlag::MemoryInfo);
}

} // namespace pbsd::stand::multiboot
