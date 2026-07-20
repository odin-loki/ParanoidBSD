module;

#include <cstdint>

export module pbsd.userland.libc.net.inet_addr;

/// inet_addr/inet_aton helpers from hbsd/src/lib/libc/inet/inet_addr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::uint32_t inet_addr(const char* cp) noexcept {
    if (cp == nullptr) {
        return 0xFFFFFFFFu;
    }
    unsigned a = 0;
    unsigned b = 0;
    unsigned c = 0;
    unsigned d = 0;
    unsigned part = 0;
    int octet = 0;
    for (; *cp != '\0'; ++cp) {
        const char ch = *cp;
        if (ch >= '0' && ch <= '9') {
            part = part * 10u + static_cast<unsigned>(ch - '0');
            if (part > 255u) {
                return 0xFFFFFFFFu;
            }
            continue;
        }
        if (ch == '.') {
            if (octet == 0) {
                a = part;
            } else if (octet == 1) {
                b = part;
            } else if (octet == 2) {
                c = part;
            } else {
                return 0xFFFFFFFFu;
            }
            part = 0;
            ++octet;
            continue;
        }
        return 0xFFFFFFFFu;
    }
    if (octet == 3) {
        d = part;
        return (a << 24) | (b << 16) | (c << 8) | d;
    }
    return 0xFFFFFFFFu;
}

} // namespace pbsd::userland::libc
