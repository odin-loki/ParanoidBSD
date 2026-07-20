module;
#include <cstdint>

export module pbsd.uda.ixl;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/ixl/ixl_pf.h / ixl_if.h (register subset).
export namespace pbsd::uda::ixl {

inline constexpr std::uint32_t kRegPfGenCtrl  = 0x000B0000;
inline constexpr std::uint32_t kRegPfIntDynCtl0 = 0x00038000;
inline constexpr std::uint32_t kRegQtxCtl     = 0x000E4000;

inline constexpr std::uint32_t kPfGenCtrlPfr = 0x00000001;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDevX710Sfp = 0x1572;

inline constexpr RegInsn kIxlInit[] = {
    {RegOp::Write32, kRegPfGenCtrl, kPfGenCtrlPfr, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kIxlReset[] = {
    {RegOp::Write32, kRegPfGenCtrl, kPfGenCtrlPfr, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ixl_x710_sfp() noexcept {
    return Descriptor{
        .name = "ixl-x710-sfp",
        .provenance = "hbsd/src/sys/dev/ixl/",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDevX710Sfp,
        .init_sequence = kIxlInit,
        .reset_sequence = kIxlReset,
    };
}

} // namespace pbsd::uda::ixl
