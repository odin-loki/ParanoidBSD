module;
#include <cstdint>

export module pbsd.kernel.link_addr;

import pbsd.core;


/// Freestanding port of `net/link_addr.c`.
export namespace pbsd::kernel::link_addr {

inline constexpr unsigned kMaxLinkAddr = 64;

[[nodiscard]] inline Status validate_len(unsigned len) noexcept {
    return len <= kMaxLinkAddr ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline bool is_broadcast(const unsigned char* addr, unsigned len) noexcept {
    if (len == 0) {
        return false;
    }
    for (unsigned i = 0; i < len; ++i) {
        if (addr[i] != 0xff) {
            return false;
        }
    }
    return true;
}

} // namespace pbsd::kernel::link_addr
