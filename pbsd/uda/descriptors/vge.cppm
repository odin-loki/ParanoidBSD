module;
#include <cstdint>

export module pbsd.uda.vge;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vge/if_vgereg.h
export namespace pbsd::uda::vge {

inline constexpr std::uint32_t kRegCrs1 = 0x09;
inline constexpr std::uint8_t  kCr1SoftReset = 0x80;

inline constexpr std::uint16_t kPciVendorVIA = 0x1106;
inline constexpr std::uint16_t kPciDevRhine2 = 0x3106;

inline constexpr RegInsn kVgeInit[] = {
    {RegOp::Write8, kRegCrs1, kCr1SoftReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVgeReset[] = {
    {RegOp::Write8, kRegCrs1, kCr1SoftReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor vge_rhine2() noexcept {
    return Descriptor{
        .name = "vge-rhine2",
        .provenance = "hbsd/src/sys/dev/vge/if_vgereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorVIA,
        .device_id = kPciDevRhine2,
        .init_sequence = kVgeInit,
        .reset_sequence = kVgeReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorVIA && device == kPciDevRhine2;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio& mem) noexcept {
    mem.write8(kRegCrs1, kCr1SoftReset);
    const auto crs = mem.read8(kRegCrs1);
    return crs != 0xFFu;
}

} // namespace pbsd::uda::vge
