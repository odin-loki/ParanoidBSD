module;
#include <cstdint>

export module pbsd.net.tcp_fastopen;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/tcp.h — TCP Fast Open cookie limits.
export namespace pbsd::net::tcp_fastopen {

inline constexpr unsigned kMinCookieLen = 4;
inline constexpr unsigned kMaxCookieLen = 16;
inline constexpr unsigned kPskLen = 16;

struct Cookie {
    unsigned char bytes[kMaxCookieLen]{};
    unsigned len{};
};

[[nodiscard]] inline Status validate_cookie_len(unsigned len) noexcept {
    if (len < kMinCookieLen || len > kMaxCookieLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_cookie(const Cookie& c) noexcept {
    return validate_cookie_len(c.len);
}

} // namespace pbsd::net::tcp_fastopen
