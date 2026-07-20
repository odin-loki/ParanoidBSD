module;
#include <cstdint>

export module pbsd.uda.fxp;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/fxp/if_fxpreg.h
export namespace pbsd::uda::fxp {

inline constexpr std::uint32_t kRegPort = 0x08;

inline constexpr std::uint32_t kPortSoftwareReset = 0;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDev82559   = 0x1229;

inline constexpr RegInsn kFxpInit[] = {
    {RegOp::Write32, kRegPort, kPortSoftwareReset, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kFxpReset[] = {
    {RegOp::Write32, kRegPort, kPortSoftwareReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor fxp_82559() noexcept {
    return Descriptor{
        .name = "fxp-82559",
        .provenance = "hbsd/src/sys/dev/fxp/if_fxpreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDev82559,
        .init_sequence = kFxpInit,
        .reset_sequence = kFxpReset,
    };
}

} // namespace pbsd::uda::fxp
