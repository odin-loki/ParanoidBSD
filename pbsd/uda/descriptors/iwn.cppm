module;
#include <cstdint>

export module pbsd.uda.iwn;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/iwn/if_iwnreg.h
export namespace pbsd::uda::iwn {

inline constexpr std::uint32_t kRegHbusTargMemRaddr = 0xA014;
inline constexpr std::uint32_t kRegHbusTargMemWaddr = 0xA018;
inline constexpr std::uint32_t kRegHbusTargMemWdata = 0xA01C;
inline constexpr std::uint32_t kRegReset            = 0x020;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDevWm4965Ag  = 0x4232;

inline constexpr RegInsn kIwnInit[] = {
    {RegOp::Write32, kRegReset, 0x1, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write32, kRegHbusTargMemWaddr, 0, 0, 0},
    {RegOp::Write32, kRegHbusTargMemWdata, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kIwnReset[] = {
    {RegOp::Write32, kRegReset, 0x1, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor iwn_wm4965ag() noexcept {
    return Descriptor{
        .name = "iwn-wm4965ag",
        .provenance = "hbsd/src/sys/dev/iwn/if_iwnreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDevWm4965Ag,
        .init_sequence = kIwnInit,
        .reset_sequence = kIwnReset,
    };
}

} // namespace pbsd::uda::iwn
