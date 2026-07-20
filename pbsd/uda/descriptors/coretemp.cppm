module;
#include <cstdint>

export module pbsd.uda.coretemp;

import pbsd.core;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/coretemp/coretemp.c
export namespace pbsd::uda::coretemp {

inline constexpr unsigned kTzZeroC = 2731;
inline constexpr unsigned kThermStatus = 0x01;
inline constexpr unsigned kThermStatusLog = 0x02;
inline constexpr unsigned kThermCritical = 0x10;
inline constexpr unsigned kTempShift = 16;
inline constexpr unsigned kTempMask = 0x7f;

[[nodiscard]] inline int decode_celsius(std::uint32_t msr) noexcept {
    const unsigned raw = (msr >> kTempShift) & kTempMask;
    return static_cast<int>(raw);
}

[[nodiscard]] inline constexpr Descriptor intel_coretemp() noexcept {
    return Descriptor{
        .name = "coretemp",
        .provenance = "hbsd/src/sys/dev/coretemp/coretemp.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x8086,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::coretemp
