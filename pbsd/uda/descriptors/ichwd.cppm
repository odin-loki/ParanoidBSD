module;
#include <cstdint>

export module pbsd.uda.ichwd;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ichwd/ichwd.h
export namespace pbsd::uda::ichwd {

inline constexpr std::uint32_t kRegTcoRld   = 0x0060;
inline constexpr std::uint32_t kRegTco1Sts  = 0x0064;
inline constexpr std::uint32_t kRegTco1Cnt  = 0x0068;

inline constexpr std::uint32_t kTcoTimeout  = 0x0008;
inline constexpr std::uint32_t kTcoTimerMask = 0x001F;
inline constexpr std::uint32_t kSmiTcoEn    = 0x2000;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDevIch7       = 0x27B8;

inline constexpr RegInsn kIchwdInit[] = {
    {RegOp::Write32, kRegTco1Sts, 0xFFFF, 0, 0},
    {RegOp::Write32, kRegTcoRld, kTcoTimerMask, 0, 0},
    {RegOp::Write32, kRegTco1Cnt, kSmiTcoEn, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kIchwdReset[] = {
    {RegOp::Write32, kRegTco1Cnt, 0, 0, 0},
    {RegOp::Write32, kRegTco1Sts, kTcoTimeout, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ichwd_ich7() noexcept {
    return Descriptor{
        .name = "ichwd-ich7",
        .provenance = "hbsd/src/sys/dev/ichwd/ichwd.h",
        .device_class = DeviceClass::Sensor,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDevIch7,
        .init_sequence = kIchwdInit,
        .reset_sequence = kIchwdReset,
    };
}

[[nodiscard]] inline constexpr bool matches_pci(std::uint16_t vendor,
                                                std::uint16_t device) noexcept {
    return vendor == kPciVendorIntel && device == kPciDevIch7;
}

[[nodiscard]] inline bool probe_mmio(SoftMmio const& mem) noexcept {
    const auto sts = mem.read32(kRegTco1Sts);
    return (sts & kTcoTimeout) == 0;
}

} // namespace pbsd::uda::ichwd
