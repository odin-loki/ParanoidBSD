module;
#include <cstdint>

export module pbsd.kernel.redzone;

import pbsd.core;

/// Freestanding port of `vm/redzone.c` — UMA redzone guard sizing.
export namespace pbsd::kernel::redzone {

inline constexpr unsigned kChSize  = 16;
inline constexpr unsigned kCfSize  = 16;
inline constexpr unsigned kHSize   = kChSize + sizeof(std::uint64_t) + 16;
inline constexpr unsigned kFSize   = kCfSize;

struct Header {
    std::uint64_t size{};
    std::uint8_t  guard[kChSize]{};
};

struct Config {
    std::uint64_t extra_mem{};
    int           panic_on_corrupt{};
};

[[nodiscard]] inline std::uint64_t roundup(std::uint64_t n) noexcept {
    if (n < kHSize) {
        n = kHSize;
    }
    if (n <= 128) return 128;
    if (n <= 256) return 256;
    if (n <= 512) return 512;
    if (n <= 1024) return 1024;
    if (n <= 2048) return 2048;
    return 4096;
}

[[nodiscard]] inline std::uint64_t size_ntor(std::uint64_t nsize) noexcept {
    return nsize + roundup(nsize) + kFSize;
}

[[nodiscard]] inline Status validate_header(const Header& hdr) noexcept {
    if (hdr.size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status check_guard(const Header& hdr) noexcept {
    for (unsigned i = 0; i < kChSize; ++i) {
        if (hdr.guard[i] != 0x5A) {
            return Status::Protocol;
        }
    }
    (void)hdr.size;
    return Status::Ok;
}

} // namespace pbsd::kernel::redzone
