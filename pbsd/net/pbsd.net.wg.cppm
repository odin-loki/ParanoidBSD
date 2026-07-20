module;
#include <cstddef>
#include <cstdint>

export module pbsd.net.wg;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/wg/if_wg.h — WireGuard key and ioctl constants.
export namespace pbsd::net::wg {

inline constexpr std::size_t kKeySize = 32;
inline constexpr unsigned kAllowedIpRemoveMe = 0x0001;
inline constexpr unsigned kAllowedIpValidFlags = kAllowedIpRemoveMe;

enum class Ioctl : unsigned int {
    Set = 210,
    Get = 211,
};

struct DataIo {
    char name[16]{};
    void* data{};
    std::size_t size{};
};

[[nodiscard]] inline Status validate_key_len(std::size_t len) noexcept {
    if (len != kKeySize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_allowedip_flags(unsigned flags) noexcept {
    if ((flags & ~kAllowedIpValidFlags) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::net::wg
