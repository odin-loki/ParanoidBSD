module;
#include <cstdint>

export module pbsd.uda.amdsbwd;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/amdsbwd/amdsbwd.c
export namespace pbsd::uda::amdsbwd {

inline constexpr std::uint16_t kPciVendorAmd = 0x1022;
inline constexpr unsigned kWatchdogSec = 60;

[[nodiscard]] inline constexpr Descriptor amd_sb_watchdog() noexcept {
    return Descriptor{
        .name = "amdsbwd",
        .provenance = "hbsd/src/sys/dev/amdsbwd/amdsbwd.c",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorAmd,
        .device_id = 0,
        .init_sequence = {},
        .reset_sequence = {},
    };
}

} // namespace pbsd::uda::amdsbwd
