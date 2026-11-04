module;
#include <cstdint>

export module pbsd.uda.uhci;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/usb/controller/uhcireg.h
export namespace pbsd::uda::uhci {

inline constexpr std::uint32_t kRegCmd   = 0x00;
inline constexpr std::uint32_t kRegSts   = 0x02;
inline constexpr std::uint32_t kRegIntr  = 0x04;

inline constexpr std::uint32_t kCmdRs      = 0x0001;
inline constexpr std::uint32_t kCmdHcreset = 0x0002;
inline constexpr std::uint32_t kStsHch     = 0x0020;

inline constexpr RegInsn kUhciInit[] = {
    {RegOp::Write32, kRegCmd, kCmdHcreset, 0, 0},
    {RegOp::CheckEq, kRegCmd, 0x0, kCmdHcreset, 1000},
    {RegOp::Write32, kRegCmd, kCmdRs, 0, 0},
    {RegOp::CheckEq, kRegSts, 0x0, kStsHch, 1000},
    {RegOp::Write32, kRegIntr, 0x000F, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kUhciReset[] = {
    {RegOp::Write32, kRegCmd, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor uhci_usb1_1() noexcept {
    return Descriptor{
        .name = "uhci-usb1.1",
        .provenance = "hbsd/src/sys/dev/usb/controller/uhcireg.h",
        .device_class = DeviceClass::Input,
        .vendor_id = 0xFFFF,
        .device_id = 0x0C03,
        .init_sequence = kUhciInit,
        .reset_sequence = kUhciReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto sts = mem.read32(kRegSts);
    return (sts & kStsHch) != 0 || (sts & 0x003Fu) == 0;
}

} // namespace pbsd::uda::uhci
