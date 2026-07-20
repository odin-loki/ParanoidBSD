module;
#include <cstdint>

export module pbsd.uda.wtap;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/wtap/if_medium.c
export namespace pbsd::uda::wtap {

inline constexpr unsigned kMaxPktBuf = 64;

struct Medium {
    bool     open{};
    unsigned pkt_count{};
};

[[nodiscard]] inline Status init_medium(Medium& md) noexcept {
    if (md.open) {
        return Status::Busy;
    }
    md.pkt_count = 0;
    md.open = true;
    return Status::Ok;
}

[[nodiscard]] inline Status deinit_medium(Medium& md) noexcept {
    if (!md.open) {
        return Status::Invalid;
    }
    md.open = false;
    md.pkt_count = 0;
    return Status::Ok;
}

[[nodiscard]] inline Status enqueue(Medium& md) noexcept {
    if (!md.open) {
        return Status::Invalid;
    }
    if (md.pkt_count >= kMaxPktBuf) {
        return Status::NoMemory;
    }
    ++md.pkt_count;
    return Status::Ok;
}

inline constexpr RegInsn kWtapInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor wtap_medium() noexcept {
    return Descriptor{
        .name = "wtap",
        .provenance = "hbsd/src/sys/dev/wtap/if_medium.c",
        .device_class = DeviceClass::Network,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kWtapInit,
        .reset_sequence = kWtapInit,
    };
}

} // namespace pbsd::uda::wtap
