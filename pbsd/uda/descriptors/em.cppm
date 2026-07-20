module;
#include <cstdint>

export module pbsd.uda.em;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/e1000/{e1000_regs.h,e1000_defines.h,e1000_hw.h,if_em.c}
export namespace pbsd::uda::em {

inline constexpr std::uint32_t kRegCtrl   = 0x00000;
inline constexpr std::uint32_t kRegStatus = 0x00008;
inline constexpr std::uint32_t kRegRctl   = 0x00100;
inline constexpr std::uint32_t kRegTctl   = 0x00400;
inline constexpr std::uint32_t kRegCtrlExt = 0x00018;

inline constexpr std::uint32_t kCtrlRst  = 0x04000000;
inline constexpr std::uint32_t kCtrlSlu  = 0x00000040;
inline constexpr std::uint32_t kRctlEn   = 0x00000002;
inline constexpr std::uint32_t kCtrlExtSpdByps = 0x00008000;

inline constexpr std::uint16_t kPciVendorIntel       = 0x8086;
inline constexpr std::uint16_t kPciDev82571EbCopper  = 0x105E;

inline constexpr RegInsn kEmInit[] = {
    {RegOp::Write32, kRegCtrl, kCtrlRst, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write32, kRegCtrlExt, kCtrlExtSpdByps, 0, 0},
    {RegOp::Write32, kRegCtrl, kCtrlSlu, 0, 0},
    {RegOp::CheckEq, kRegStatus, 0x0, 0x2, 1000},
    {RegOp::Write32, kRegRctl, kRctlEn, 0, 0},
    {RegOp::Write32, kRegTctl, 0x000003FA, 0, 0},
    {RegOp::Write32, kRegTctl, 0x000003FB, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kEmReset[] = {
    {RegOp::Write32, kRegCtrl, kCtrlRst, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor em_82571_eb_copper() noexcept {
    return Descriptor{
        .name = "em-82571eb-copper",
        .provenance = "hbsd/src/sys/dev/e1000/e1000_hw.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDev82571EbCopper,
        .init_sequence = kEmInit,
        .reset_sequence = kEmReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorIntel && device == kPciDev82571EbCopper;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto status = mem.read32(kRegStatus);
    return (status & 0x2u) != 0 || status == 0;
}

} // namespace pbsd::uda::em
