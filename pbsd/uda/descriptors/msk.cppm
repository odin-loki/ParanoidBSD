module;
#include <cstdint>

export module pbsd.uda.msk;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/msk/if_mskreg.h — Marvell Yukon2 init.
export namespace pbsd::uda::msk {

inline constexpr std::uint32_t kRegCtst   = 0x0004;
inline constexpr std::uint32_t kRegIsrc   = 0x0008;
inline constexpr std::uint32_t kRegImsk   = 0x000C;
inline constexpr std::uint32_t kRegRap    = 0x0000;

inline constexpr std::uint32_t kCtstInit  = 0x0001;
inline constexpr std::uint32_t kCtstRun   = 0x0002;

inline constexpr std::uint16_t kPciVendorMarvell = 0x11AB;
inline constexpr std::uint16_t kPciDev8021cu     = 0x4340;

inline constexpr RegInsn kMskInit[] = {
    {RegOp::Write16, kRegCtst, kCtstInit, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write32, kRegImsk, 0x0, 0, 0},
    {RegOp::Write16, kRegCtst, kCtstRun, 0, 0},
    {RegOp::CheckEq, kRegCtst, kCtstRun, 0x0003, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kMskReset[] = {
    {RegOp::Write16, kRegCtst, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor msk_yukon2_8021cu() noexcept {
    return Descriptor{
        .name = "msk-yukon2-8021cu",
        .provenance = "hbsd/src/sys/dev/msk/if_mskreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorMarvell,
        .device_id = kPciDev8021cu,
        .init_sequence = kMskInit,
        .reset_sequence = kMskReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorMarvell && device == kPciDev8021cu;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    return (mem.read16(kRegCtst) & kCtstRun) != 0 || mem.read16(kRegCtst) == 0;
}

} // namespace pbsd::uda::msk
