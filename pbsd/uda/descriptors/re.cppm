module;
#include <cstdint>

export module pbsd.uda.re;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/re/if_re.c, hbsd/src/sys/dev/rl/if_rlreg.h
export namespace pbsd::uda::re {

inline constexpr std::uint32_t kRegCommand = 0x0037;
inline constexpr std::uint32_t kRegCfg1    = 0x0052;
inline constexpr std::uint32_t kRegRxCfg   = 0x0044;
inline constexpr std::uint32_t kRegTxCfg   = 0x0040;

inline constexpr std::uint32_t kCmdReset   = 0x0010;
inline constexpr std::uint32_t kCmdRxEnb   = 0x0008;
inline constexpr std::uint32_t kCmdTxEnb   = 0x0004;
inline constexpr std::uint32_t kCfg1Drvload = 0x20;
inline constexpr std::uint32_t kRxCfgAap   = 0x00000001;
inline constexpr std::uint32_t kTxCfgIfg     = 0x3F000000;

inline constexpr std::uint16_t kPciVendorRealtek = 0x10EC;
inline constexpr std::uint16_t kPciDev8168       = 0x8168;

inline constexpr RegInsn kReInit[] = {
    {RegOp::Write32, kRegCommand, kCmdReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Write32, kRegCfg1, kCfg1Drvload, 0, 0},
    {RegOp::Write32, kRegRxCfg, kRxCfgAap, 0, 0},
    {RegOp::Write32, kRegTxCfg, kTxCfgIfg, 0, 0},
    {RegOp::CheckEq, kRegCommand, 0x0, kCmdReset, 1000},
    {RegOp::Write32, kRegCommand, kCmdRxEnb | kCmdTxEnb, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kReReset[] = {
    {RegOp::Write32, kRegCommand, kCmdReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor re_rtl8168() noexcept {
    return Descriptor{
        .name = "re-rtl8168",
        .provenance = "hbsd/src/sys/dev/rl/if_rlreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorRealtek,
        .device_id = kPciDev8168,
        .init_sequence = kReInit,
        .reset_sequence = kReReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorRealtek && device == kPciDev8168;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto cfg1 = mem.read32(kRegCfg1);
    return cfg1 != 0 && cfg1 != 0xFFFFFFFFu;
}

} // namespace pbsd::uda::re
