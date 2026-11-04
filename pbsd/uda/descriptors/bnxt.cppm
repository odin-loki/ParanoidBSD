module;
#include <cstdint>

export module pbsd.uda.bnxt;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/bnxt/if_bnxt.h, bnxt_hwrm.h
export namespace pbsd::uda::bnxt {

inline constexpr std::uint32_t kRegBar0GrcCfg = 0x00000000;
inline constexpr std::uint32_t kRegBar0Doorbell = 0x00100000;
inline constexpr std::uint32_t kRegGrcCfgEnable = 0x00000001;

inline constexpr std::uint16_t kPciVendorBroadcom = 0x14E4;
inline constexpr std::uint16_t kPciDevNetXtreme  = 0x16D8;

inline constexpr RegInsn kBnxtInit[] = {
    {RegOp::Write32, kRegBar0GrcCfg, kRegGrcCfgEnable, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Write32, kRegBar0Doorbell, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kBnxtReset[] = {
    {RegOp::Write32, kRegBar0GrcCfg, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor bnxt_netxtreme() noexcept {
    return Descriptor{
        .name = "bnxt-netxtreme",
        .provenance = "hbsd/src/sys/dev/bnxt/if_bnxt.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorBroadcom,
        .device_id = kPciDevNetXtreme,
        .init_sequence = kBnxtInit,
        .reset_sequence = kBnxtReset,
    };
}

} // namespace pbsd::uda::bnxt
