module;
#include <cstdint>

export module pbsd.uda.amdsmb;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/amdsmb/amdsmb.c
export namespace pbsd::uda::amdsmb {

inline constexpr std::uint16_t kPciVendorAmd = 0x1022;
inline constexpr std::uint32_t kRegControl = 0x00;

[[nodiscard]] inline constexpr Descriptor amd_fch_smb() noexcept {
    return Descriptor{
        .name = "amdsmb",
        .provenance = "hbsd/src/sys/dev/amdsmb/amdsmb.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorAmd,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::amdsmb
