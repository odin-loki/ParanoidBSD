module;
#include <cstdint>

export module pbsd.net.netisr;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/netisr.h — deferred network ISR protocol IDs.
export namespace pbsd::net::netisr {

inline constexpr int kProtoIp          = 1;
inline constexpr int kProtoIgmp        = 2;
inline constexpr int kProtoRoute       = 3;
inline constexpr int kProtoArp         = 4;
inline constexpr int kProtoEther       = 5;
inline constexpr int kProtoIpv6        = 6;
inline constexpr int kProtoIpDirect    = 9;
inline constexpr int kProtoIpv6Direct  = 10;

enum class Policy : unsigned char {
    Source = 1,
    Flow   = 2,
    Cpu    = 3,
};

enum class Dispatch : unsigned char {
    Default  = 0,
    Deferred = 1,
    Hybrid   = 2,
    Direct   = 3,
};

struct HandlerStub {
    int       proto{};
    Policy    policy{Policy::Source};
    Dispatch  dispatch{Dispatch::Default};
    unsigned  queued{};
    unsigned  dispatched{};
};

[[nodiscard]] inline Status validate_proto(int proto) noexcept {
    if (proto <= 0 || proto >= 32) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status register_handler(HandlerStub& h, int proto) noexcept {
    if (validate_proto(proto) != Status::Ok) {
        return Status::Invalid;
    }
    h.proto = proto;
    return Status::Ok;
}

[[nodiscard]] inline Status queue(HandlerStub& h) noexcept {
    if (h.proto == 0) {
        return Status::Invalid;
    }
    ++h.queued;
    return Status::Ok;
}

[[nodiscard]] inline Status dispatch(HandlerStub& h) noexcept {
    if (h.queued == 0) {
        return Status::Invalid;
    }
    --h.queued;
    ++h.dispatched;
    return Status::Ok;
}

} // namespace pbsd::net::netisr
