module;
#include <cstdint>

export module pbsd.uda.amdgpio;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/amdgpio/amdgpio.c
export namespace pbsd::uda::amdgpio {

inline constexpr std::uint32_t kRegPinCtrl = 0x00;
inline constexpr std::uint32_t kPinInput = 0x00000001;
inline constexpr std::uint16_t kPciVendorAmd = 0x1022;

[[nodiscard]] inline constexpr Descriptor amd_fch_gpio() noexcept {
    return Descriptor{
        .name = "amdgpio",
        .provenance = "hbsd/src/sys/dev/amdgpio/amdgpio.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorAmd,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::amdgpio
