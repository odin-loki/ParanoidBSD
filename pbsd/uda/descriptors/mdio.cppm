module;
#include <cstdint>

export module pbsd.uda.mdio;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/dev/mdio/mdio.h
export namespace pbsd::uda::mdio {

inline constexpr int kDevAddrNone = -1;
inline constexpr std::uint8_t kPhyMin = 0;
inline constexpr std::uint8_t kPhyMax = 31;
inline constexpr std::uint8_t kRegMax = 31;

inline constexpr RegInsn kMdioInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kMdioReset[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor mdio_bus() noexcept {
    return Descriptor{
        .name = "mdio",
        .provenance = "hbsd/src/sys/dev/mdio/mdio.h",
        .device_class = DeviceClass::Network,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kMdioInit,
        .reset_sequence = kMdioReset,
    };
}

[[nodiscard]] inline Status validate_phy_reg(int phy, int reg) noexcept {
    if (phy < kPhyMin || phy > kPhyMax || reg < 0 || reg > kRegMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_ext_reg(int phy, int devad, int reg) noexcept {
    if (validate_phy_reg(phy, reg) != Status::Ok) {
        return Status::Invalid;
    }
    if (devad < 0 || devad > 31) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::uda::mdio
