module;
#include <cstdint>

export module pbsd.net.divert;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/netinet/ip_divert.h — ipdivert(4) socket metadata.
export namespace pbsd::net::divert {

inline constexpr unsigned short kProto = 258; // IPPROTO_DIVERT

struct Stats {
    unsigned long long diverted{};
    unsigned long long noport{};
    unsigned long long outbound{};
    unsigned long long inbound{};
};

/// Kernel→userland: sin_port holds matching rule (host order).
/// Userland→kernel: sin_port holds restart-rule minus one (host order).
[[nodiscard]] inline Status validate_restart_rule(unsigned short rule_minus_one) noexcept {
    if (rule_minus_one == 0xffff) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline unsigned short encode_restart_port(unsigned rule) noexcept {
    return static_cast<unsigned short>(rule - 1);
}

[[nodiscard]] inline unsigned decode_restart_rule(unsigned short port) noexcept {
    return static_cast<unsigned>(port) + 1;
}

[[nodiscard]] inline Status validate_iface_name(char const* name) noexcept {
    if (name == nullptr) {
        return Status::Invalid;
    }
    unsigned len = 0;
    while (name[len] != '\0') {
        if (++len > 7) {
            return Status::Invalid;
        }
    }
    return Status::Ok;
}

} // namespace pbsd::net::divert
