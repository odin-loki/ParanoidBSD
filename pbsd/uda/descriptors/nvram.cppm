module;
#include <cstdint>

export module pbsd.uda.nvram;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/nvram/nvram.c
export namespace pbsd::uda::nvram {

inline constexpr unsigned kCmosStart  = 14;
inline constexpr unsigned kCmosEnd    = 128;
inline constexpr unsigned kCmosSize   = kCmosEnd - kCmosStart;
inline constexpr unsigned kChecksumLo = 32;
inline constexpr unsigned kChecksumHi = 33;

struct SoftState {
    bool     attached{};
    unsigned offset{};
};

[[nodiscard]] inline Status validate_offset(unsigned offset, unsigned len) noexcept {
    if (offset + len > kCmosSize) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status attach(SoftState& sc) noexcept {
    if (sc.attached) {
        return Status::Busy;
    }
    sc.offset = 0;
    sc.attached = true;
    return Status::Ok;
}

[[nodiscard]] inline Status read_byte(SoftState& sc, unsigned offset,
                                      unsigned char& value) noexcept {
    if (!sc.attached || validate_offset(offset, 1) != Status::Ok) {
        return Status::Invalid;
    }
    value = 0;
    return Status::Ok;
}

inline constexpr RegInsn kNvramInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor nvram_cmos() noexcept {
    return Descriptor{
        .name = "nvram",
        .provenance = "hbsd/src/sys/dev/nvram/nvram.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kNvramInit,
        .reset_sequence = kNvramInit,
    };
}

} // namespace pbsd::uda::nvram
