module;
#include <cstdint>

export module pbsd.uda.axgbe;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/axgbe/if_axgreg.h
export namespace pbsd::uda::axgbe {

inline constexpr std::uint32_t kRegDmaMode = 0x1000;
inline constexpr std::uint32_t kDmaSoftReset = 0x00000001;
inline constexpr std::uint16_t kPciVendorAmd = 0x1022;
inline constexpr std::uint16_t kPciDev1458 = 0x1458;

inline constexpr RegInsn kAxgbeInit[] = {
    {RegOp::Write32, kRegDmaMode, kDmaSoftReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor axgbe_10g() noexcept {
    return Descriptor{
        .name = "axgbe-10g",
        .provenance = "hbsd/src/sys/dev/axgbe/if_axgreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorAmd,
        .device_id = kPciDev1458,
        .init_sequence = kAxgbeInit,
        .reset_sequence = kAxgbeInit,
    };
}

} // namespace pbsd::uda::axgbe
