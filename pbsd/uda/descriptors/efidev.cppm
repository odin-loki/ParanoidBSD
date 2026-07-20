module;
#include <cstdint>

export module pbsd.uda.efidev;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/efidev/efidev.c
export namespace pbsd::uda::efidev {

inline constexpr std::uint64_t kGuidEndEntire = 0x0000000010000000ULL;
inline constexpr std::uint16_t kDevPathPci = 0x01;
inline constexpr std::uint16_t kDevPathAcpi = 0x02;

[[nodiscard]] inline constexpr Descriptor efi_runtime() noexcept {
    return Descriptor{
        .name = "efidev",
        .provenance = "hbsd/src/sys/dev/efidev/efidev.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::efidev
