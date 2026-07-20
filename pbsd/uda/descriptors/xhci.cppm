module;
#include <cstdint>

export module pbsd.uda.xhci;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/usb/controller/xhcireg.h
export namespace pbsd::uda::xhci {

inline constexpr std::uint32_t kRegUsbcmd = 0x00;
inline constexpr std::uint32_t kRegUsbsts = 0x04;
inline constexpr std::uint32_t kRegDnctrl = 0x14;
inline constexpr std::uint32_t kRegConfig = 0x38;

inline constexpr std::uint32_t kCmdRs    = 0x00000001;
inline constexpr std::uint32_t kCmdHcrst = 0x00000002;
inline constexpr std::uint32_t kStsHch   = 0x00000001;
inline constexpr std::uint32_t kStsCnr   = 0x00000800;
inline constexpr std::uint32_t kDnctrlDen0 = 0x00000001;

inline constexpr RegInsn kXhciInit[] = {
    {RegOp::Write32, kRegUsbcmd, kCmdHcrst, 0, 0},
    {RegOp::CheckEq, kRegUsbcmd, 0x0, kCmdHcrst, 1000},
    {RegOp::Write32, kRegUsbcmd, kCmdRs, 0, 0},
    {RegOp::CheckEq, kRegUsbsts, 0x0, kStsHch, 1000},
    {RegOp::Write32, kRegDnctrl, kDnctrlDen0, 0, 0},
    {RegOp::Write32, kRegConfig, 0x1, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kXhciReset[] = {
    {RegOp::Write32, kRegUsbcmd, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor xhci_usb3_1_0() noexcept {
    return Descriptor{
        .name = "xhci-usb3-1.0",
        .provenance = "hbsd/src/sys/dev/usb/controller/xhcireg.h",
        .device_class = DeviceClass::Input,
        .vendor_id = 0xFFFF,
        .device_id = 0x0C03,
        .init_sequence = kXhciInit,
        .reset_sequence = kXhciReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto sts = mem.read32(kRegUsbsts);
    return (sts & kStsCnr) == 0;
}

} // namespace pbsd::uda::xhci
