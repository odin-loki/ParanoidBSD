module;
#include <cstdint>

export module pbsd.uda.wpi;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/wpi/if_wpireg.h
export namespace pbsd::uda::wpi {

inline constexpr std::uint32_t kRegReset = 0x020;
inline constexpr std::uint32_t kResetSw = 0x00000080;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDev4965agn = 0x4222;

inline constexpr RegInsn kWpiInit[] = {
    {RegOp::Write32, kRegReset, kResetSw, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor wpi_4965agn() noexcept {
    return Descriptor{
        .name = "wpi-4965agn",
        .provenance = "hbsd/src/sys/dev/wpi/if_wpireg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDev4965agn,
        .init_sequence = kWpiInit,
        .reset_sequence = kWpiInit,
    };
}

} // namespace pbsd::uda::wpi
