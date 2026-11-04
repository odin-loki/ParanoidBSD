module;
#include <cstdint>

export module pbsd.uda.enic;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/enic/if_enicreg.h
export namespace pbsd::uda::enic {

inline constexpr std::uint32_t kRegDevCmd = 0x0000;
inline constexpr std::uint32_t kDevCmdReset = 0x00000001;
inline constexpr std::uint16_t kPciVendorCisco = 0x1137;
inline constexpr std::uint16_t kPciDev0043 = 0x0043;

inline constexpr RegInsn kEnicInit[] = {
    {RegOp::Write32, kRegDevCmd, kDevCmdReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor enic_vnic() noexcept {
    return Descriptor{
        .name = "enic-vnic",
        .provenance = "hbsd/src/sys/dev/enic/if_enicreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorCisco,
        .device_id = kPciDev0043,
        .init_sequence = kEnicInit,
        .reset_sequence = kEnicInit,
    };
}

} // namespace pbsd::uda::enic
