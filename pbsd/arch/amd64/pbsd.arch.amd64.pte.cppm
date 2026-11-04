module;
#include <cstdint>

export module pbsd.arch.amd64.pte;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/amd64/include/pte.h
export namespace pbsd::arch::amd64::pte {

inline constexpr std::uint64_t kValid = 0x001;
inline constexpr std::uint64_t kWrite = 0x002;
inline constexpr std::uint64_t kUser = 0x004;
inline constexpr std::uint64_t kAccessed = 0x020;
inline constexpr std::uint64_t kDirty = 0x040;
inline constexpr std::uint64_t kLargePage = 0x080;
inline constexpr std::uint64_t kGlobal = 0x100;
inline constexpr std::uint64_t kNoExecute = 1ull << 63;

inline constexpr std::uint64_t kEptRead = 0x001;
inline constexpr std::uint64_t kEptWrite = 0x002;
inline constexpr std::uint64_t kEptExecute = 0x004;

[[nodiscard]] inline bool is_present(std::uint64_t pte) noexcept {
    return (pte & kValid) != 0;
}

[[nodiscard]] inline Status validate_pte(std::uint64_t pte) noexcept {
    if (!is_present(pte)) {
        return Status::Ok;
    }
    if ((pte & kLargePage) != 0 && (pte & kNoExecute) == 0) {
        return Status::Ok;
    }
    return Status::Ok;
}

} // namespace pbsd::arch::amd64::pte
