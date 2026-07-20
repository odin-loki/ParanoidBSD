module;
#include <cstdint>

export module pbsd.uda.bce;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/bce/if_bcereg.h
export namespace pbsd::uda::bce {

inline constexpr std::uint32_t kRegCpReset = 0x00;
inline constexpr std::uint32_t kCpReset = 0x00000020;

inline constexpr std::uint16_t kPciVendorBroadcom = 0x14E4;
inline constexpr std::uint16_t kPciDevNetXtreme2 = 0x1639;

inline constexpr RegInsn kBceInit[] = {
    {RegOp::Write32, kRegCpReset, kCpReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kBceReset[] = {
    {RegOp::Write32, kRegCpReset, kCpReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor bce_netxtreme2() noexcept {
    return Descriptor{
        .name = "bce-netxtreme2",
        .provenance = "hbsd/src/sys/dev/bce/if_bcereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorBroadcom,
        .device_id = kPciDevNetXtreme2,
        .init_sequence = kBceInit,
        .reset_sequence = kBceReset,
    };
}

} // namespace pbsd::uda::bce
