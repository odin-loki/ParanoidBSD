module;
#include <cstdint>

export module pbsd.uda.bwn;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/bwn/if_bwnreg.h
export namespace pbsd::uda::bwn {

inline constexpr std::uint32_t kIoctlPhyReset = 0x0008;
inline constexpr std::uint32_t kIoctlPhyClockEnable = 0x0004;

inline constexpr std::uint16_t kPciVendorBroadcom = 0x14E4;
inline constexpr std::uint16_t kPciDevBcm4306 = 0x4306;

inline constexpr RegInsn kBwnInit[] = {
    {RegOp::Write32, 0x00, kIoctlPhyClockEnable, 0, 0},
    {RegOp::Write32, 0x00, kIoctlPhyReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor bwn_bcm4306() noexcept {
    return Descriptor{
        .name = "bwn-bcm4306",
        .provenance = "hbsd/src/sys/dev/bwn/if_bwnreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorBroadcom,
        .device_id = kPciDevBcm4306,
        .init_sequence = kBwnInit,
        .reset_sequence = kBwnInit,
    };
}

} // namespace pbsd::uda::bwn
