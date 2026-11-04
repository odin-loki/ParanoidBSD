module;
#include <cstdint>

export module pbsd.uda.ahci;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ahci/ahci.h — AHCI 1.3.1 register map.
export namespace pbsd::uda::ahci {

inline constexpr std::uint32_t kRegCap  = 0x00;
inline constexpr std::uint32_t kRegGhc  = 0x04;
inline constexpr std::uint32_t kRegIs   = 0x08;
inline constexpr std::uint32_t kRegPi   = 0x0C;
inline constexpr std::uint32_t kRegVs   = 0x10;
inline constexpr std::uint32_t kRegCap2 = 0x24;

inline constexpr std::uint32_t kGhcAe  = 0x80000000;
inline constexpr std::uint32_t kGhcIe  = 0x00000002;
inline constexpr std::uint32_t kGhcHr  = 0x00000001;

inline constexpr std::uint32_t kCapNpMask = 0x0000001F;

inline constexpr RegInsn kAhciInit[] = {
    {RegOp::Write32, kRegGhc, kGhcAe, 0, 0},
    {RegOp::Write32, kRegGhc, kGhcAe | kGhcHr, 0, 0},
    {RegOp::CheckEq, kRegGhc, 0x0, kGhcHr, 1000},
    {RegOp::Write32, kRegGhc, kGhcAe | kGhcIe, 0, 0},
    {RegOp::Write32, kRegIs, 0xFFFFFFFF, 0, 0},
    {RegOp::Read32, kRegPi, 0, 0, 0},
    {RegOp::Write32, kRegGhc, kGhcAe, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAhciReset[] = {
    {RegOp::Write32, kRegGhc, kGhcAe | kGhcHr, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ahci_sata_1_3() noexcept {
    return Descriptor{
        .name = "ahci-sata-1.3",
        .provenance = "hbsd/src/sys/dev/ahci/ahci.h",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x8086,
        .device_id = 0xFFFF,
        .init_sequence = kAhciInit,
        .reset_sequence = kAhciReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto cap = mem.read32(kRegCap);
    const auto np = cap & kCapNpMask;
    return np > 0 && np <= 32;
}

} // namespace pbsd::uda::ahci
