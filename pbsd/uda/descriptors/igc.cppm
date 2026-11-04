module;
#include <cstdint>

export module pbsd.uda.igc;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/igc/{igc_regs.h,igc_defines.h,igc_hw.h}
export namespace pbsd::uda::igc {

inline constexpr std::uint32_t kRegCtrl   = 0x00000;
inline constexpr std::uint32_t kRegStatus = 0x00008;
inline constexpr std::uint32_t kRegRctl  = 0x00100;
inline constexpr std::uint32_t kRegTctl  = 0x00400;

inline constexpr std::uint32_t kCtrlRst = 0x04000000;
inline constexpr std::uint32_t kCtrlSlu = 0x00000040;
inline constexpr std::uint32_t kRctlEn  = 0x00000002;

inline constexpr std::uint16_t kPciVendorIntel  = 0x8086;
inline constexpr std::uint16_t kPciDevI225Lm      = 0x15F2;

inline constexpr RegInsn kIgcInit[] = {
    {RegOp::Write32, kRegCtrl, kCtrlRst, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write32, kRegCtrl, kCtrlSlu, 0, 0},
    {RegOp::CheckEq, kRegStatus, 0x0, 0x2, 1000},
    {RegOp::Write32, kRegRctl, kRctlEn, 0, 0},
    {RegOp::Write32, kRegTctl, 0x000003FA, 0, 0},
    {RegOp::Write32, kRegTctl, 0x000003FB, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kIgcReset[] = {
    {RegOp::Write32, kRegCtrl, kCtrlRst, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor igc_i225_lm() noexcept {
    return Descriptor{
        .name = "igc-i225-lm",
        .provenance = "hbsd/src/sys/dev/igc/igc_hw.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDevI225Lm,
        .init_sequence = kIgcInit,
        .reset_sequence = kIgcReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorIntel && device == kPciDevI225Lm;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto status = mem.read32(kRegStatus);
    return (status & 0x2u) != 0 || status == 0;
}

} // namespace pbsd::uda::igc
