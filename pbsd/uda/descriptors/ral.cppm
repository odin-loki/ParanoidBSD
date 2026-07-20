module;
#include <cstdint>

export module pbsd.uda.ral;

import pbsd.core;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/ral/rt2560reg.h, if_ral_pci.c
export namespace pbsd::uda::ral {

inline constexpr std::uint32_t kRegMacStatus = 0x0400;
inline constexpr std::uint32_t kRegPciCfg    = 0x0500;
inline constexpr std::uint32_t kMacStatusRfOn = 0x00000001;

inline constexpr std::uint16_t kPciVendorRalink = 0x1814;
inline constexpr std::uint16_t kPciDevRt2560   = 0x0301;

inline constexpr RegInsn kRalInit[] = {
    {RegOp::Write32, kRegPciCfg, 0x1, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Write32, kRegMacStatus, kMacStatusRfOn, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kRalReset[] = {
    {RegOp::Write32, kRegMacStatus, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor ral_rt2560() noexcept {
    return Descriptor{
        .name = "ral-rt2560",
        .provenance = "hbsd/src/sys/dev/ral/rt2560reg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorRalink,
        .device_id = kPciDevRt2560,
        .init_sequence = kRalInit,
        .reset_sequence = kRalReset,
    };
}

} // namespace pbsd::uda::ral
