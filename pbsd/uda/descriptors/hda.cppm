module;
#include <cstdint>

export module pbsd.uda.hda;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/sound/pci/hda/{hdac_reg.h,hdac.h}
export namespace pbsd::uda::hda {

inline constexpr std::uint32_t kRegGctl    = 0x00008;
inline constexpr std::uint32_t kRegIntctl  = 0x00020;
inline constexpr std::uint32_t kRegCorbctl = 0x0004C;
inline constexpr std::uint32_t kRegRirbctl = 0x0005C;
inline constexpr std::uint32_t kRegSdnctl  = 0x00080;

inline constexpr std::uint32_t kGctlCrst      = 0x00000001;
inline constexpr std::uint32_t kGctlFcNtrl      = 0x00000002;
inline constexpr std::uint32_t kCorbctlCorbrun  = 0x00000002;
inline constexpr std::uint32_t kCorbctlCmeie    = 0x00000001;
inline constexpr std::uint32_t kRirbctlRirbdmaen = 0x00000002;
inline constexpr std::uint32_t kRirbctlRintctl   = 0x00000001;
inline constexpr std::uint32_t kSdnctlRun       = 0x00000001;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDev82801F     = 0x2668;

inline constexpr RegInsn kHdaInit[] = {
    {RegOp::Write32, kRegGctl, 0, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write32, kRegGctl, kGctlCrst, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write32, kRegGctl, kGctlCrst | kGctlFcNtrl, 0, 0},
    {RegOp::Write32, kRegCorbctl, kCorbctlCmeie | kCorbctlCorbrun, 0, 0},
    {RegOp::Write32, kRegRirbctl, kRirbctlRintctl | kRirbctlRirbdmaen, 0, 0},
    {RegOp::Write32, kRegSdnctl, kSdnctlRun, 0, 0},
    {RegOp::Write32, kRegIntctl, 0xC0000000u, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kHdaReset[] = {
    {RegOp::Write32, kRegCorbctl, 0, 0, 0},
    {RegOp::Write32, kRegRirbctl, 0, 0, 0},
    {RegOp::Write32, kRegGctl, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor hda_intel_82801f() noexcept {
    return Descriptor{
        .name = "hda-intel-82801f",
        .provenance = "hbsd/src/sys/dev/sound/pci/hda/hdac_reg.h",
        .device_class = DeviceClass::Input,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDev82801F,
        .init_sequence = kHdaInit,
        .reset_sequence = kHdaReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorIntel && device == kPciDev82801F;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto gcap = mem.read32(0x00);
    return gcap != 0 && gcap != 0xFFFFFFFFu;
}

} // namespace pbsd::uda::hda
