module;
#include <cstdint>

export module pbsd.net.pfsync;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/pfvar.h, netinet/in.h — PFSYNC protocol constants.
export namespace pbsd::net::pfsync {

inline constexpr unsigned char kIpProto = 240;
inline constexpr unsigned kGroupAddr = 0xe00000f0; // 224.0.0.240
inline constexpr unsigned kModVer = 1;
inline constexpr unsigned kMinVer = 1;
inline constexpr unsigned kMaxVer = 1;

enum class Action : unsigned char {
    Inserts = 0,
    InsAck  = 1,
    UpdC    = 2,
    UpdAck  = 3,
    DelC    = 4,
    DelAck  = 5,
    Clear   = 6,
    ClearAck = 7,
};

[[nodiscard]] inline Status validate_version(unsigned ver) noexcept {
    if (ver < kMinVer || ver > kMaxVer) {
        return Status::Protocol;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_action(Action a) noexcept {
    switch (a) {
    case Action::Inserts:
    case Action::InsAck:
    case Action::UpdC:
    case Action::UpdAck:
    case Action::DelC:
    case Action::DelAck:
    case Action::Clear:
    case Action::ClearAck:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

} // namespace pbsd::net::pfsync
