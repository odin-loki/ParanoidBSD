module;

#include <cstdint>

export module pbsd.userland.rtld.reloc;

/// relocation type constants from hbsd/src/libexec/rtld-elf/amd64/reloc.c
export namespace pbsd::userland::rtld {

inline constexpr unsigned kRelocRelative = 8;
inline constexpr unsigned kRelocGlobDat = 6;
inline constexpr unsigned kRelocJumpSlot = 7;
inline constexpr unsigned kRelocTlsTpoFF64 = 18;

struct RelocEntry {
    std::uintptr_t offset{0};
    std::uintptr_t info{0};
    std::intptr_t addend{0};
};

[[nodiscard]] inline unsigned reloc_type(const RelocEntry& r) noexcept {
    return static_cast<unsigned>(r.info & 0xffffffffU);
}

} // namespace pbsd::userland::rtld
