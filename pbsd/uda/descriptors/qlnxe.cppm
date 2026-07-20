module;
#include <cstdint>

export module pbsd.uda.qlnxe;

import pbsd.core;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/qlnx/qlnxe.h, if_qlnxe.c
export namespace pbsd::uda::qlnxe {

inline constexpr std::uint32_t kRegBar0Ctrl = 0x00000000;
inline constexpr std::uint32_t kRegBar0Status = 0x00000008;
inline constexpr std::uint32_t kCtrlEnable = 0x00000001;

inline constexpr std::uint16_t kPciVendorQlogic = 0x1077;
inline constexpr std::uint16_t kPciDevQle8242   = 0x1634;

inline constexpr RegInsn kQlnxeInit[] = {
    {RegOp::Write32, kRegBar0Ctrl, 0, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write32, kRegBar0Ctrl, kCtrlEnable, 0, 0},
    {RegOp::CheckEq, kRegBar0Status, 0x1, 0x1, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kQlnxeReset[] = {
    {RegOp::Write32, kRegBar0Ctrl, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor qlnxe_qle8242() noexcept {
    return Descriptor{
        .name = "qlnxe-qle8242",
        .provenance = "hbsd/src/sys/dev/qlnx/qlnxe.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorQlogic,
        .device_id = kPciDevQle8242,
        .init_sequence = kQlnxeInit,
        .reset_sequence = kQlnxeReset,
    };
}

} // namespace pbsd::uda::qlnxe
