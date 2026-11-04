module;
#include <cstdint>

export module pbsd.uda.ale;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ale/if_alereg.h
export namespace pbsd::uda::ale {

inline constexpr std::uint32_t kRegMacCfg = 0x1480;

inline constexpr std::uint32_t kMacCfgTxEnb = 0x00000001;
inline constexpr std::uint32_t kMacCfgRxEnb = 0x00000002;

inline constexpr std::uint16_t kPciVendorAtheros = 0x1969;
inline constexpr std::uint16_t kPciDevAr81xx    = 0x1026;

inline constexpr RegInsn kAleInit[] = {
    {RegOp::Write32, kRegMacCfg, kMacCfgTxEnb | kMacCfgRxEnb, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAleReset[] = {
    {RegOp::Write32, kRegMacCfg, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ale_ar81xx() noexcept {
    return Descriptor{
        .name = "ale-ar81xx",
        .provenance = "hbsd/src/sys/dev/ale/if_alereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorAtheros,
        .device_id = kPciDevAr81xx,
        .init_sequence = kAleInit,
        .reset_sequence = kAleReset,
    };
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto cfg = mem.read32(kRegMacCfg);
    return (cfg & (kMacCfgTxEnb | kMacCfgRxEnb)) != 0 || cfg == 0;
}

} // namespace pbsd::uda::ale
