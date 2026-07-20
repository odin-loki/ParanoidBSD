module;
#include <cstdint>

export module pbsd.uda.amdtemp;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/amdtemp/amdtemp.c
export namespace pbsd::uda::amdtemp {

inline constexpr unsigned kOffsetC = 49;
inline constexpr unsigned kMask = 0xff;

[[nodiscard]] inline int decode_offset(std::uint32_t reg) noexcept {
    return static_cast<int>((reg >> 21) & kMask) - static_cast<int>(kOffsetC);
}

[[nodiscard]] inline constexpr Descriptor amd_k8_temp() noexcept {
    return Descriptor{
        .name = "amdtemp-k8",
        .provenance = "hbsd/src/sys/dev/amdtemp/amdtemp.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0x1022,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::amdtemp
