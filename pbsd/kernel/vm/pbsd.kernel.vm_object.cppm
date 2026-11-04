module;
#include <cstdint>

export module pbsd.kernel.vm_object;

import pbsd.core;

/// Freestanding port of `vm_object.h` flags + pindex/offset algebra.
export namespace pbsd::kernel::vm_object {

inline constexpr unsigned kPageShift = 12u;
inline constexpr unsigned kPageSize  = 1u << kPageShift;

inline constexpr unsigned kObjFictitious  = 0x00000001u;
inline constexpr unsigned kObjUnmanaged   = 0x00000002u;
inline constexpr unsigned kObjPopulate    = 0x00000004u;
inline constexpr unsigned kObjDead        = 0x00000008u;
inline constexpr unsigned kObjAnon        = 0x00000010u;
inline constexpr unsigned kObjShadowlist  = 0x00000100u;
inline constexpr unsigned kObjSwap        = 0x00000200u;
inline constexpr unsigned kObjColored     = 0x00001000u;
inline constexpr unsigned kObjOnemapping  = 0x00002000u;

inline constexpr unsigned kObjpcSync   = 0x1u;
inline constexpr unsigned kObjpcInval = 0x2u;

[[nodiscard]] constexpr std::uint64_t idx_to_off(std::uint64_t idx) noexcept {
    return idx << kPageShift;
}

[[nodiscard]] constexpr std::uint64_t off_to_idx(std::uint64_t off) noexcept {
    return off >> kPageShift;
}

[[nodiscard]] constexpr bool is_anonymous(unsigned flags) noexcept {
    return (flags & kObjAnon) != 0;
}

[[nodiscard]] constexpr bool is_dead(unsigned flags) noexcept {
    return (flags & kObjDead) != 0;
}

[[nodiscard]] constexpr bool supports_populate(unsigned flags) noexcept {
    return (flags & kObjPopulate) != 0;
}

struct VmObjectHeader {
    unsigned      flags{};
    std::uint64_t size{};
    unsigned char type{};
};

/// `vm_object_reference` — reject dead objects.
[[nodiscard]] constexpr Status reference_ok(const VmObjectHeader& obj) noexcept {
    if (is_dead(obj.flags)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

/// `vm_object_truncate` — size must be page-aligned.
[[nodiscard]] constexpr Status validate_truncate_size(std::uint64_t new_size) noexcept {
    if ((new_size & (kPageSize - 1)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::vm_object
