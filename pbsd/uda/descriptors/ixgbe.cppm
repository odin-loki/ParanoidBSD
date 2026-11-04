module;
#include <cstdint>

export module pbsd.uda.ixgbe;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ixgbe/ixgbe_type.h
export namespace pbsd::uda::ixgbe {

inline constexpr std::uint32_t kRegCtrl      = 0x00000;
inline constexpr std::uint32_t kRegStatus    = 0x00008;
inline constexpr std::uint32_t kRegRdrxctl  = 0x02F00;
inline constexpr std::uint32_t kRegFctrl    = 0x05080;

inline constexpr std::uint32_t kCtrlRst     = 0x04000000;
inline constexpr std::uint32_t kCtrlSlu      = 0x00000040;
inline constexpr std::uint32_t kRdrxctlCrcstrip = 0x00000002;
inline constexpr std::uint32_t kFctrlBam     = 0x00000400;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDev82599Sfp = 0x10FB;

inline constexpr RegInsn kIxgbeInit[] = {
    {RegOp::Write32, kRegCtrl, kCtrlRst, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write32, kRegCtrl, kCtrlSlu, 0, 0},
    {RegOp::CheckEq, kRegStatus, 0x0, 0x2, 1000},
    {RegOp::Write32, kRegRdrxctl, kRdrxctlCrcstrip, 0, 0},
    {RegOp::Write32, kRegFctrl, kFctrlBam, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kIxgbeReset[] = {
    {RegOp::Write32, kRegCtrl, kCtrlRst, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ixgbe_82599_sfp() noexcept {
    return Descriptor{
        .name = "ixgbe-82599-sfp",
        .provenance = "hbsd/src/sys/dev/ixgbe/ixgbe_type.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDev82599Sfp,
        .init_sequence = kIxgbeInit,
        .reset_sequence = kIxgbeReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorIntel && device == kPciDev82599Sfp;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto status = mem.read32(kRegStatus);
    return (status & 0x2u) != 0 || status == 0;
}

} // namespace pbsd::uda::ixgbe
