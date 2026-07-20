module;
#include <cstdint>

export module pbsd.uda.sis;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/sis/if_sisreg.h
export namespace pbsd::uda::sis {

inline constexpr std::uint32_t kRegCsr = 0x00;
inline constexpr std::uint32_t kCsrReset = 0x00000100;
inline constexpr std::uint32_t kCsrRxEnable = 0x00000004;
inline constexpr std::uint32_t kCsrTxEnable = 0x00000001;

inline constexpr std::uint16_t kPciVendorSiS = 0x1039;
inline constexpr std::uint16_t kPciDev900 = 0x0900;

inline constexpr RegInsn kSisInit[] = {
    {RegOp::Write32, kRegCsr, kCsrReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write32, kRegCsr, kCsrRxEnable | kCsrTxEnable, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kSisReset[] = {
    {RegOp::Write32, kRegCsr, kCsrReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor sis_900() noexcept {
    return Descriptor{
        .name = "sis-900",
        .provenance = "hbsd/src/sys/dev/sis/if_sisreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorSiS,
        .device_id = kPciDev900,
        .init_sequence = kSisInit,
        .reset_sequence = kSisReset,
    };
}

} // namespace pbsd::uda::sis
