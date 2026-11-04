module;
#include <cstdint>

export module pbsd.uda.vr;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/vr/if_vrreg.h
export namespace pbsd::uda::vr {

inline constexpr std::uint32_t kRegCr1 = 0x09;
inline constexpr std::uint8_t  kCr1Reset = 0x80;

inline constexpr std::uint16_t kPciVendorVia = 0x1106;
inline constexpr std::uint16_t kPciDevRhine = 0x3065;

inline constexpr RegInsn kVrInit[] = {
    {RegOp::Write8, kRegCr1, kCr1Reset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kVrReset[] = {
    {RegOp::Write8, kRegCr1, kCr1Reset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor vr_rhine() noexcept {
    return Descriptor{
        .name = "vr-rhine",
        .provenance = "hbsd/src/sys/dev/vr/if_vrreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorVia,
        .device_id = kPciDevRhine,
        .init_sequence = kVrInit,
        .reset_sequence = kVrReset,
    };
}

} // namespace pbsd::uda::vr
