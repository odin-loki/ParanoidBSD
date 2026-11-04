module;
#include <cstdint>

export module pbsd.uda.oce;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/oce/if_oce.h, oce_hw.h
export namespace pbsd::uda::oce {

inline constexpr std::uint32_t kRegPciCfg = 0x00000000;
inline constexpr std::uint32_t kRegDevCtrl = 0x00000004;
inline constexpr std::uint32_t kDevCtrlReset = 0x00000001;

inline constexpr std::uint16_t kPciVendorEmulex = 0x10DF;
inline constexpr std::uint16_t kPciDevOneConnect = 0xE220;

inline constexpr RegInsn kOceInit[] = {
    {RegOp::Write32, kRegDevCtrl, kDevCtrlReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write32, kRegPciCfg, 0x1, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kOceReset[] = {
    {RegOp::Write32, kRegDevCtrl, kDevCtrlReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor oce_oneconnect() noexcept {
    return Descriptor{
        .name = "oce-oneconnect",
        .provenance = "hbsd/src/sys/dev/oce/if_oce.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorEmulex,
        .device_id = kPciDevOneConnect,
        .init_sequence = kOceInit,
        .reset_sequence = kOceReset,
    };
}

} // namespace pbsd::uda::oce
