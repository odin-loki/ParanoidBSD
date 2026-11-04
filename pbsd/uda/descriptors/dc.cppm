module;
#include <cstdint>

export module pbsd.uda.dc;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/dc/if_dcreg.h, if_dc.c
export namespace pbsd::uda::dc {

inline constexpr std::uint32_t kRegBusctl  = 0x00;
inline constexpr std::uint32_t kRegNetcfg  = 0x30;

inline constexpr std::uint32_t kBusctlReset = 0x00000001;
inline constexpr std::uint32_t kNetcfgRxOn  = 0x00000002;
inline constexpr std::uint32_t kNetcfgTxOn  = 0x00002000;

inline constexpr std::uint16_t kPciVendorDec    = 0x1011;
inline constexpr std::uint16_t kPciDev21143     = 0x0019;
inline constexpr std::uint16_t kPciVendorAdmtek = 0x1317;
inline constexpr std::uint16_t kPciDevAl981     = 0x0981;

inline constexpr RegInsn kDcInit[] = {
    {RegOp::Write32, kRegBusctl, kBusctlReset, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write32, kRegNetcfg, kNetcfgRxOn | kNetcfgTxOn, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kDcReset[] = {
    {RegOp::Write32, kRegBusctl, kBusctlReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor dc_21143() noexcept {
    return Descriptor{
        .name = "dc-21143",
        .provenance = "hbsd/src/sys/dev/dc/if_dcreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorDec,
        .device_id = kPciDev21143,
        .init_sequence = kDcInit,
        .reset_sequence = kDcReset,
    };
}

[[nodiscard]] inline constexpr Descriptor dc_al981() noexcept {
    return Descriptor{
        .name = "dc-al981",
        .provenance = "hbsd/src/sys/dev/dc/if_dcreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorAdmtek,
        .device_id = kPciDevAl981,
        .init_sequence = kDcInit,
        .reset_sequence = kDcReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return (vendor == kPciVendorDec && device == kPciDev21143)
        || (vendor == kPciVendorAdmtek && device == kPciDevAl981);
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto cfg = mem.read32(kRegNetcfg);
    return cfg != 0 && cfg != 0xFFFFFFFFu;
}

} // namespace pbsd::uda::dc
