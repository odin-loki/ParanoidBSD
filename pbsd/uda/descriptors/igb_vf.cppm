module;
#include <cstdint>

export module pbsd.uda.igb_vf;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/e1000/{e1000_vf.h,e1000_regs.h,if_em.c}
export namespace pbsd::uda::igb_vf {

inline constexpr std::uint32_t kRegCtrl   = 0x00000;
inline constexpr std::uint32_t kRegStatus = 0x00008;
inline constexpr std::uint32_t kRegEims   = 0x01524;
inline constexpr std::uint32_t kRegEimc   = 0x01528;
inline constexpr std::uint32_t kRegTxdctl0 = 0x03828;
inline constexpr std::uint32_t kRegRxdctl0 = 0x02828;

inline constexpr std::uint32_t kCtrlRst = 0x04000000;
inline constexpr std::uint32_t kCtrlSlu  = 0x00000040;
inline constexpr std::uint32_t kTxdctlQueueEnable = 0x02000000;
inline constexpr std::uint32_t kRxdctlQueueEnable = 0x02000000;

inline constexpr std::uint16_t kPciVendorIntel   = 0x8086;
inline constexpr std::uint16_t kPciDev82576Vf     = 0x10CA;
inline constexpr std::uint16_t kPciDevI350Vf       = 0x1520;

inline constexpr RegInsn kIgbVfInit[] = {
    {RegOp::Write32, kRegCtrl, kCtrlRst, 0, 0},
    {RegOp::WaitUs, 0, 200, 0, 0},
    {RegOp::Write32, kRegEimc, 0xFFFFFFFF, 0, 0},
    {RegOp::Write32, kRegCtrl, kCtrlSlu, 0, 0},
    {RegOp::Write32, kRegTxdctl0, kTxdctlQueueEnable, 0, 0},
    {RegOp::Write32, kRegRxdctl0, kRxdctlQueueEnable, 0, 0},
    {RegOp::Write32, kRegEims, 0x1, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kIgbVfReset[] = {
    {RegOp::Write32, kRegEimc, 0xFFFFFFFF, 0, 0},
    {RegOp::Write32, kRegCtrl, kCtrlRst, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor igb_82576_vf() noexcept {
    return Descriptor{
        .name = "igb-82576-vf",
        .provenance = "hbsd/src/sys/dev/e1000/e1000_vf.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDev82576Vf,
        .init_sequence = kIgbVfInit,
        .reset_sequence = kIgbVfReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorIntel
        && (device == kPciDev82576Vf || device == kPciDevI350Vf);
}

} // namespace pbsd::uda::igb_vf
