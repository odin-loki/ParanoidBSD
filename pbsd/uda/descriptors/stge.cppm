module;
#include <cstdint>

export module pbsd.uda.stge;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/stge/if_stgereg.h, if_stge.c
export namespace pbsd::uda::stge {

inline constexpr std::uint32_t kRegAsicCtrl = 0x30;
inline constexpr std::uint32_t kRegMacCtrl  = 0x6C;

inline constexpr std::uint32_t kAcGlobalReset = 0x00010000;
inline constexpr std::uint32_t kAcDma         = 0x00080000;
inline constexpr std::uint32_t kAcNetwork     = 0x00200000;
inline constexpr std::uint32_t kMcTxEnable    = 0x01000000;
inline constexpr std::uint32_t kMcRxEnable    = 0x02000000;

inline constexpr std::uint16_t kPciVendorSundance = 0x13F0;
inline constexpr std::uint16_t kPciDevSt1023      = 0x1023;

inline constexpr RegInsn kStgeInit[] = {
    {RegOp::Write32, kRegAsicCtrl, kAcGlobalReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Write32, kRegAsicCtrl, kAcDma | kAcNetwork, 0, 0},
    {RegOp::Write32, kRegMacCtrl, kMcTxEnable | kMcRxEnable, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kStgeReset[] = {
    {RegOp::Write32, kRegAsicCtrl, kAcGlobalReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor stge_tc9021() noexcept {
    return Descriptor{
        .name = "stge-tc9021",
        .provenance = "hbsd/src/sys/dev/stge/if_stgereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorSundance,
        .device_id = kPciDevSt1023,
        .init_sequence = kStgeInit,
        .reset_sequence = kStgeReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorSundance && device == kPciDevSt1023;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio& mem) noexcept {
    mem.write32(kRegAsicCtrl, kAcGlobalReset);
    const auto mac = mem.read32(kRegMacCtrl);
    return mac != 0 && mac != 0xFFFFFFFFu;
}

} // namespace pbsd::uda::stge
