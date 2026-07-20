module;
#include <cstdint>

export module pbsd.uda.dcons;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/dcons/dcons.c
export namespace pbsd::uda::dcons {

inline constexpr unsigned kGenShift = 24;
inline constexpr unsigned kPosMask  = 0x00ffffffu;

struct Channel {
    std::uint32_t* ptr{nullptr};
    unsigned      gen{};
    unsigned      pos{};
};

[[nodiscard]] inline bool is_char(const Channel& ch) noexcept {
    if (ch.ptr == nullptr) {
        return false;
    }
    const std::uint32_t raw = *ch.ptr;
    const unsigned gen = raw >> kGenShift;
    const unsigned pos = raw & kPosMask;
    return gen != ch.gen || pos != ch.pos;
}

[[nodiscard]] inline Status advance(Channel& ch) noexcept {
    if (ch.ptr == nullptr) {
        return Status::Invalid;
    }
    const std::uint32_t raw = *ch.ptr;
    ch.gen = raw >> kGenShift;
    ch.pos = raw & kPosMask;
    return Status::Ok;
}

inline constexpr RegInsn kDconsInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor dcons_generic() noexcept {
    return Descriptor{
        .name = "dcons",
        .provenance = "hbsd/src/sys/dev/dcons/dcons.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kDconsInit,
        .reset_sequence = kDconsInit,
    };
}

} // namespace pbsd::uda::dcons
