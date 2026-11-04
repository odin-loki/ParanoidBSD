module;
#include <cstdint>

export module pbsd.uda.tun;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/net/if_tun.c
export namespace pbsd::uda::tun {

inline constexpr unsigned kMaxUnit = 256u;

struct SoftState {
    int           unit{-1};
    bool          point_to_point{};
    bool          attached{};
    std::uint64_t packets{};
};

[[nodiscard]] inline Status attach(SoftState& sc, int unit) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    if (unit < 0 || static_cast<unsigned>(unit) >= kMaxUnit) {
        return Status::Invalid;
    }
    sc.unit = unit;
    sc.point_to_point = true;
    sc.packets = 0;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status transmit(SoftState& sc) noexcept {
    if (!sc.attached) {
        return Status::Invalid;
    }
    ++sc.packets;
    return Status::Ok;
}

inline constexpr RegInsn kTunInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor tun_dev() noexcept {
    return Descriptor{
        .name = "tun",
        .provenance = "hbsd/src/sys/net/if_tun.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kTunInit,
        .reset_sequence = kTunInit,
    };
}

} // namespace pbsd::uda::tun
