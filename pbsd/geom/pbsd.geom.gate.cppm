module;
#include <cstdint>

export module pbsd.geom.gate;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/geom/gate/g_gate.h — GEOM GATE provider constants.
export namespace pbsd::geom::gate {

inline constexpr unsigned kVersion = 3;
inline constexpr unsigned kMaxQueueSize = 4096;
inline constexpr unsigned kMaxUnit = 256;
inline constexpr char kProviderPrefix[] = "g_gate";

enum class Cmd : unsigned char {
    Create = 0,
    Destroy = 1,
    Start = 2,
    Stop = 3,
};

enum class Flag : unsigned int {
    ReadOnly = 0x00000001,
    CanFree  = 0x00000002,
    NeedFree = 0x00000004,
};

struct Unit {
    unsigned unit{};
    unsigned flags{};
    unsigned queue_size{};
    bool     active{};
};

[[nodiscard]] inline Status validate_cmd(Cmd c) noexcept {
    switch (c) {
    case Cmd::Create:
    case Cmd::Destroy:
    case Cmd::Start:
    case Cmd::Stop:
        return Status::Ok;
    default:
        return Status::Invalid;
    }
}

[[nodiscard]] inline Status validate_unit(unsigned unit) noexcept {
    if (unit >= kMaxUnit) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_queue_size(unsigned size) noexcept {
    if (size == 0 || size > kMaxQueueSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status init_unit(Unit& u, unsigned unit, unsigned queue_size) noexcept {
    if (validate_unit(unit) != Status::Ok || validate_queue_size(queue_size) != Status::Ok) {
        return Status::Invalid;
    }
    u.unit = unit;
    u.queue_size = queue_size;
    u.flags = 0;
    u.active = false;
    return Status::Ok;
}

[[nodiscard]] inline Status start(Unit& u) noexcept {
    if (u.active) {
        return Status::Busy;
    }
    u.active = true;
    return Status::Ok;
}

[[nodiscard]] inline Status stop(Unit& u) noexcept {
    if (!u.active) {
        return Status::Invalid;
    }
    u.active = false;
    return Status::Ok;
}

} // namespace pbsd::geom::gate
