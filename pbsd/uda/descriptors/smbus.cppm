module;
#include <cstdint>

export module pbsd.uda.smbus;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ichsmb/{ichsmb_reg.h,ichsmb_pci.c}, hbsd/src/sys/dev/smbus/smbus.h
export namespace pbsd::uda::smbus {

inline constexpr std::uint32_t kRegHstSta = 0x00;
inline constexpr std::uint32_t kRegHstCnt = 0x02;
inline constexpr std::uint32_t kRegHstCmd = 0x03;
inline constexpr std::uint32_t kRegXmitSlva = 0x04;

inline constexpr std::uint32_t kHstStaHostBusy = 0x01;
inline constexpr std::uint32_t kHstStaIntr     = 0x02;
inline constexpr std::uint32_t kHstCntStart    = 0x40;
inline constexpr std::uint32_t kHstCntIntren   = 0x01;
inline constexpr std::uint32_t kHstCntSmbCmdByte = 0x04;
inline constexpr std::uint32_t kXmitSlvaWrite  = 0x00;

inline constexpr std::uint8_t kAddrMin = 0x10;
inline constexpr std::uint8_t kAddrMax = 0x70;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDev82801Aa    = 0x2413;

inline constexpr RegInsn kSmbusInit[] = {
    {RegOp::Write32, kRegHstSta, 0xFF, 0, 0},
    {RegOp::Write32, kRegHstCnt, kHstCntIntren, 0, 0},
    {RegOp::Write32, kRegXmitSlva, kXmitSlvaWrite, 0, 0},
    {RegOp::Write32, kRegHstCmd, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kSmbusReadByte[] = {
    {RegOp::Write32, kRegHstCnt, kHstCntStart | kHstCntIntren, 0, 0},
    {RegOp::CheckEq, kRegHstSta, kHstStaIntr, kHstStaIntr, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kSmbusReset[] = {
    {RegOp::Write32, kRegHstCnt, 0, 0, 0},
    {RegOp::Write32, kRegHstSta, 0xFF, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ichsmb_82801aa() noexcept {
    return Descriptor{
        .name = "ichsmb-82801aa",
        .provenance = "hbsd/src/sys/dev/ichsmb/ichsmb_reg.h",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDev82801Aa,
        .init_sequence = kSmbusInit,
        .reset_sequence = kSmbusReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorIntel && device == kPciDev82801Aa;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return (mem.read32(kRegHstSta) & kHstStaHostBusy) == 0;
}

} // namespace pbsd::uda::smbus
