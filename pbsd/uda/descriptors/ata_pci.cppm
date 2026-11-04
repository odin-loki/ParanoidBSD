module;
#include <cstdint>

export module pbsd.uda.ata_pci;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ata/{ata-pci.h,ata-all.h} — legacy PCI ATA channel.
export namespace pbsd::uda::ata_pci {

inline constexpr std::uint32_t kBar0Stride = 0x08;

inline constexpr std::uint32_t kRegData    = 0x00;
inline constexpr std::uint32_t kRegFeature = 0x01;
inline constexpr std::uint32_t kRegCommand = 0x07;
inline constexpr std::uint32_t kRegStatus  = 0x07;
inline constexpr std::uint32_t kRegControl = 0x0E;

inline constexpr std::uint32_t kCmdIdentify = 0xEC;
inline constexpr std::uint32_t kStatusBusy  = 0x80;
inline constexpr std::uint32_t kStatusDrq   = 0x08;
inline constexpr std::uint32_t kControlSrst = 0x04;

inline constexpr RegInsn kAtaPciInit[] = {
    {RegOp::Write8, kRegControl, kControlSrst, 0, 0},
    {RegOp::WaitUs, 0, 5, 0, 0},
    {RegOp::Write8, kRegControl, 0x0, 0, 0},
    {RegOp::WaitUs, 0, 5, 0, 0},
    {RegOp::Write8, kRegCommand, kCmdIdentify, 0, 0},
    {RegOp::CheckEq, kRegStatus, kStatusDrq, kStatusDrq | kStatusBusy, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAtaPciReset[] = {
    {RegOp::Write8, kRegControl, kControlSrst, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ata_pci_channel0() noexcept {
    return Descriptor{
        .name = "ata-pci-ch0",
        .provenance = "hbsd/src/sys/dev/ata/ata-pci.h",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x8086,
        .device_id = 0x24D1,
        .init_sequence = kAtaPciInit,
        .reset_sequence = kAtaPciReset,
    };
}

[[nodiscard]] inline bool probe_status(SoftMmio const& mem) noexcept {
    const auto st = mem.read8(kRegStatus);
    return st != 0xFF && st != 0x00;
}

} // namespace pbsd::uda::ata_pci
