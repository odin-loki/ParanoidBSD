module;
#include <cstdint>

export module pbsd.uda.sdhci;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/sdhci/sdhci.h — SDHCI host init.
export namespace pbsd::uda::sdhci {

inline constexpr std::uint32_t kRegDmaAddress    = 0x00;
inline constexpr std::uint32_t kRegBlockSize     = 0x04;
inline constexpr std::uint32_t kRegPresentState  = 0x24;
inline constexpr std::uint32_t kRegPowerControl  = 0x29;
inline constexpr std::uint32_t kRegClockControl  = 0x2C;
inline constexpr std::uint32_t kRegSoftwareReset = 0x2F;

inline constexpr std::uint32_t kPowerOn330      = 0x0E;
inline constexpr std::uint32_t kClockIntEn      = 0x0001;
inline constexpr std::uint32_t kClockCardEn     = 0x0004;
inline constexpr std::uint32_t kResetAll        = 0x01;
inline constexpr std::uint32_t kPresentCardStable = 0x00030000;

inline constexpr RegInsn kSdhciInit[] = {
    {RegOp::Write8,  kRegSoftwareReset, kResetAll, 0, 0},
    {RegOp::WaitUs,  0, 10, 0, 0},
    {RegOp::Write8,  kRegPowerControl, kPowerOn330, 0, 0},
    {RegOp::Write16, kRegClockControl, kClockIntEn, 0, 0},
    {RegOp::CheckEq, kRegClockControl, kClockIntEn, 0x0002, 1000},
    {RegOp::Write16, kRegClockControl, kClockIntEn | kClockCardEn, 0, 0},
    {RegOp::CheckEq, kRegPresentState, kPresentCardStable, kPresentCardStable, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kSdhciReset[] = {
    {RegOp::Write8, kRegSoftwareReset, kResetAll, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor sdhci_pci_host() noexcept {
    return Descriptor{
        .name = "sdhci-pci-host",
        .provenance = "hbsd/src/sys/dev/sdhci/sdhci.h",
        .device_class = DeviceClass::Block,
        .vendor_id = 0x8086,
        .device_id = 0xFFFF,
        .init_sequence = kSdhciInit,
        .reset_sequence = kSdhciReset,
    };
}

[[nodiscard]] inline bool probe_present(SoftMmio const& mem) noexcept {
    const auto ps = mem.read32(kRegPresentState);
    return (ps & kPresentCardStable) == kPresentCardStable;
}

} // namespace pbsd::uda::sdhci
