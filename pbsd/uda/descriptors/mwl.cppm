module;
#include <cstdint>

export module pbsd.uda.mwl;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/mwl/mwlreg.h
export namespace pbsd::uda::mwl {

inline constexpr unsigned kMcastMax = 32;
inline constexpr std::uint32_t kRegReset = 0x00;
inline constexpr std::uint32_t kResetMagic = 0x00000001;

inline constexpr std::uint16_t kPciVendorMarvell = 0x11AB;
inline constexpr std::uint16_t kPciDev88w8363 = 0x1aAA;

inline constexpr RegInsn kMwlInit[] = {
    {RegOp::Write32, kRegReset, kResetMagic, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mwl_88w8363() noexcept {
    return Descriptor{
        .name = "mwl-88w8363",
        .provenance = "hbsd/src/sys/dev/mwl/mwlreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorMarvell,
        .device_id = kPciDev88w8363,
        .init_sequence = kMwlInit,
        .reset_sequence = kMwlInit,
    };
}

} // namespace pbsd::uda::mwl
