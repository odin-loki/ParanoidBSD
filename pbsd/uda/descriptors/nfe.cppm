module;
#include <cstdint>

export module pbsd.uda.nfe;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/nfe/if_nfereg.h
export namespace pbsd::uda::nfe {

inline constexpr std::uint32_t kRegMacReset = 0x03c;
inline constexpr std::uint32_t kMacResetMagic = 0x00f3;

inline constexpr std::uint16_t kPciVendorNvidia = 0x10DE;
inline constexpr std::uint16_t kPciDevMcp55Lan = 0x0373;

inline constexpr RegInsn kNfeInit[] = {
    {RegOp::Write32, kRegMacReset, kMacResetMagic, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kNfeReset[] = {
    {RegOp::Write32, kRegMacReset, kMacResetMagic, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor nfe_mcp55() noexcept {
    return Descriptor{
        .name = "nfe-mcp55",
        .provenance = "hbsd/src/sys/dev/nfe/if_nfereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorNvidia,
        .device_id = kPciDevMcp55Lan,
        .init_sequence = kNfeInit,
        .reset_sequence = kNfeReset,
    };
}

} // namespace pbsd::uda::nfe
