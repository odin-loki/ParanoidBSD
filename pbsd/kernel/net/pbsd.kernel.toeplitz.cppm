module;
#include <cstdint>

export module pbsd.kernel.toeplitz;

import pbsd.core;

/// Freestanding port of `net/toeplitz.c` — RSS Toeplitz hash for NIC queues.
export namespace pbsd::kernel::toeplitz {

inline constexpr unsigned kKeySize = 40;

[[nodiscard]] inline std::uint32_t hash(unsigned keylen, std::uint8_t const* key,
                                        unsigned datalen,
                                        std::uint8_t const* data) noexcept {
    if (key == nullptr || data == nullptr || keylen == 0 || datalen == 0) {
        return 0;
    }
    std::uint32_t hash_val = 0;
    std::uint32_t v = (static_cast<std::uint32_t>(key[0]) << 24) |
                      (static_cast<std::uint32_t>(key[1]) << 16) |
                      (static_cast<std::uint32_t>(key[2]) << 8) |
                      static_cast<std::uint32_t>(key[3]);
    for (unsigned i = 0; i < datalen; ++i) {
        for (unsigned b = 0; b < 8; ++b) {
            if (data[i] & (1u << (7 - b))) {
                hash_val ^= v;
            }
            v <<= 1;
            if ((i + 4) < kKeySize && (key[i + 4] & (1u << (7 - b)))) {
                v |= 1u;
            }
        }
    }
    return hash_val;
}

[[nodiscard]] inline Status validate_keylen(unsigned keylen) noexcept {
    if (keylen == 0 || keylen > kKeySize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::toeplitz
