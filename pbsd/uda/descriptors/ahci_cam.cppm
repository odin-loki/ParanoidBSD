module;
#include <cstdint>

export module pbsd.uda.ahci_cam;

import pbsd.core;
export import pbsd.uda.schema;
export import pbsd.uda.ahci;

/// PROVENANCE: hbsd/src/sys/dev/ahci/{ahci.h,ahci.c} — CAM port command path.
export namespace pbsd::uda::ahci_cam {

inline constexpr std::uint32_t kPortStride = 0x80;
inline constexpr std::uint32_t kPort0Base  = 0x100;

inline constexpr std::uint32_t kPClb  = 0x00;
inline constexpr std::uint32_t kPClbu = 0x04;
inline constexpr std::uint32_t kPFb   = 0x08;
inline constexpr std::uint32_t kPFbu  = 0x0c;
inline constexpr std::uint32_t kPIe  = 0x14;
inline constexpr std::uint32_t kPCmd  = 0x18;
inline constexpr std::uint32_t kPSsts = 0x28;

inline constexpr std::uint32_t kPCmdSt  = 0x00000001;
inline constexpr std::uint32_t kPCmdSud = 0x00000002;
inline constexpr std::uint32_t kPCmdFre = 0x00000010;
inline constexpr std::uint32_t kPIeMask = 0x00DC0000;

inline constexpr std::uint32_t kSsDetPhyOnline = 0x00000003;

inline constexpr RegInsn kAhciCamPortInit[] = {
    {RegOp::Write32, kPort0Base + kPClb,  0x1000, 0, 0},
    {RegOp::Write32, kPort0Base + kPClbu, 0x0, 0, 0},
    {RegOp::Write32, kPort0Base + kPFb,   0x2000, 0, 0},
    {RegOp::Write32, kPort0Base + kPFbu,  0x0, 0, 0},
    {RegOp::Write32, kPort0Base + kPIe,   kPIeMask, 0, 0},
    {RegOp::Write32, kPort0Base + kPCmd,  kPCmdFre | kPCmdSud, 0, 0},
    {RegOp::CheckEq, kPort0Base + kPSsts, kSsDetPhyOnline, 0x0000000F, 1000},
    {RegOp::Write32, kPort0Base + kPCmd,  kPCmdSt | kPCmdFre | kPCmdSud, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAhciCamPortStop[] = {
    {RegOp::Write32, kPort0Base + kPCmd, 0x0, 0, 0},
    {RegOp::Write32, kPort0Base + kPIe, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr std::uint32_t kPCi   = 0x38;
inline constexpr std::uint32_t kPCiAct = 0x00000001;

inline constexpr RegInsn kAhciCamIssueCmd[] = {
    {RegOp::Write32, kPort0Base + kPCi, kPCiAct, 0, 0},
    {RegOp::CheckEq, kPort0Base + kPSsts, kSsDetPhyOnline, 0x0000000F, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ahci_cam_port0() noexcept {
    return Descriptor{
        .name = "ahci-cam-port0",
        .provenance = "hbsd/src/sys/dev/ahci/ahci.h",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x8086,
        .device_id = 0xFFFF,
        .init_sequence = kAhciCamPortInit,
        .reset_sequence = kAhciCamPortStop,
    };
}

[[nodiscard]] inline constexpr std::uint32_t port_mmio_base(unsigned port) noexcept {
    return kPort0Base + port * kPortStride;
}

[[nodiscard]] inline bool probe_port(SoftMmio const& mem, unsigned port) noexcept {
    const auto base = port_mmio_base(port);
    const auto ssts = mem.read32(base + kPSsts);
    return (ssts & 0x0000000Fu) != 0;
}

[[nodiscard]] constexpr Status validate_port(unsigned port) noexcept {
    return port < 32 ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::uda::ahci_cam
