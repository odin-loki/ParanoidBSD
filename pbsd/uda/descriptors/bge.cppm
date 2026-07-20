module;
#include <cstdint>

export module pbsd.uda.bge;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/bge/if_bgereg.h — BCM570x init.
export namespace pbsd::uda::bge {

inline constexpr std::uint32_t kRegMacMode   = 0x0400;
inline constexpr std::uint32_t kRegModeCtl    = 0x6800;
inline constexpr std::uint32_t kRegMiscCfg    = 0x6804;
inline constexpr std::uint32_t kRegMemWinBase = 0x7C00;

inline constexpr std::uint32_t kMacModeEnable = 0x00000001;
inline constexpr std::uint32_t kModeCtlHostSendBds = 0x00020000;

inline constexpr std::uint16_t kPciVendorBroadcom = 0x14E4;
inline constexpr std::uint16_t kPciDev5700         = 0x1644;

inline constexpr RegInsn kBgeInit[] = {
    {RegOp::Write32, kRegMiscCfg, 0x00000001, 0, 0},
    {RegOp::Write32, kRegModeCtl, kModeCtlHostSendBds, 0, 0},
    {RegOp::Write32, kRegMacMode, kMacModeEnable, 0, 0},
    {RegOp::CheckEq, kRegMacMode, kMacModeEnable, kMacModeEnable, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kBgeReset[] = {
    {RegOp::Write32, kRegMacMode, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor bge_5700_copper() noexcept {
    return Descriptor{
        .name = "bge-5700-copper",
        .provenance = "hbsd/src/sys/dev/bge/if_bgereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorBroadcom,
        .device_id = kPciDev5700,
        .init_sequence = kBgeInit,
        .reset_sequence = kBgeReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                              std::uint16_t device) noexcept {
    return vendor == kPciVendorBroadcom && device == kPciDev5700;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return mem.read32(kRegMacMode) != 0xFFFFFFFFu;
}

} // namespace pbsd::uda::bge
