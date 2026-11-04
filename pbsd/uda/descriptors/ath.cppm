module;
#include <cstdint>

export module pbsd.uda.ath;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/ath/athreg.h, if_ath_pci.c
export namespace pbsd::uda::ath {

inline constexpr std::uint32_t kRegMacVersion = 0x0000;
inline constexpr std::uint32_t kRegMacReset   = 0x0040;
inline constexpr std::uint32_t kMacResetSelf  = 0x00000001;

inline constexpr std::uint16_t kPciVendorAtheros = 0x168C;
inline constexpr std::uint16_t kPciDevAr9280     = 0x002A;

inline constexpr RegInsn kAthInit[] = {
    {RegOp::Write32, kRegMacReset, kMacResetSelf, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::CheckEq, kRegMacVersion, 0, 0xFFFF, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAthReset[] = {
    {RegOp::Write32, kRegMacReset, kMacResetSelf, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ath_ar9280() noexcept {
    return Descriptor{
        .name = "ath-ar9280",
        .provenance = "hbsd/src/sys/dev/ath/athreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorAtheros,
        .device_id = kPciDevAr9280,
        .init_sequence = kAthInit,
        .reset_sequence = kAthReset,
    };
}

} // namespace pbsd::uda::ath
