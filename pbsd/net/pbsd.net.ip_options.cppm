module;
#include <cstdint>

export module pbsd.net.ip_options;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_var.h — IPv4 option classes.
export namespace pbsd::net::ip_options {

inline constexpr unsigned kMaxLen = 40;

enum class Class : unsigned char {
    Control = 0,
    Reserved = 1,
    Measurement = 2,
};

enum class Number : unsigned char {
    Eol = 0,
    Nop = 1,
    Lsrr = 131,
    Ssrr = 137,
    RecordRoute = 7,
    Timestamp = 68,
};

[[nodiscard]] inline Status validate_len(unsigned len) noexcept {
    if (len > kMaxLen) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_number(Number n) noexcept {
    switch (n) {
    case Number::Eol:
    case Number::Nop:
    case Number::Lsrr:
    case Number::Ssrr:
    case Number::RecordRoute:
    case Number::Timestamp:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::net::ip_options
