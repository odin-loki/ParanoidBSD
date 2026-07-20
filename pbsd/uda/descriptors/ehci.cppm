module;
#include <cstdint>

export module pbsd.uda.ehci;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/usb/controller/ehcireg.h
export namespace pbsd::uda::ehci {

inline constexpr std::uint32_t kRegUsbcmd = 0x00;
inline constexpr std::uint32_t kRegUsbsts = 0x04;
inline constexpr std::uint32_t kRegUsbintr = 0x08;

inline constexpr std::uint32_t kCmdRs      = 0x00000001;
inline constexpr std::uint32_t kCmdHcreset = 0x00000002;
inline constexpr std::uint32_t kStsHch     = 0x00001000;

inline constexpr RegInsn kEhciInit[] = {
    {RegOp::Write32, kRegUsbcmd, kCmdHcreset, 0, 0},
    {RegOp::CheckEq, kRegUsbcmd, 0x0, kCmdHcreset, 1000},
    {RegOp::Write32, kRegUsbcmd, kCmdRs, 0, 0},
    {RegOp::CheckEq, kRegUsbsts, 0x0, kStsHch, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kEhciReset[] = {
    {RegOp::Write32, kRegUsbcmd, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ehci_usb2_0() noexcept {
    return Descriptor{
        .name = "ehci-usb2.0",
        .provenance = "hbsd/src/sys/dev/usb/controller/ehcireg.h",
        .device_class = DeviceClass::Input,
        .vendor_id = 0xFFFF,
        .device_id = 0x0C03,
        .init_sequence = kEhciInit,
        .reset_sequence = kEhciReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto sts = mem.read32(kRegUsbsts);
    return (sts & kStsHch) != 0u;
}

} // namespace pbsd::uda::ehci
