module;
#include <cstdint>

export module pbsd.uda.iwm;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/iwm/if_iwmreg.h
export namespace pbsd::uda::iwm {

inline constexpr std::uint32_t kRegHbusTargWaddr = 0xA018;
inline constexpr std::uint32_t kRegHbusTargWdata = 0xA01C;
inline constexpr std::uint32_t kRegReset         = 0x020;

inline constexpr std::uint16_t kPciVendorIntel = 0x8086;
inline constexpr std::uint16_t kPciDevWm7260   = 0x08B1;

inline constexpr RegInsn kIwmInit[] = {
    {RegOp::Write32, kRegReset, 0x1, 0, 0},
    {RegOp::WaitUs, 0, 10, 0, 0},
    {RegOp::Write32, kRegHbusTargWaddr, 0, 0, 0},
    {RegOp::Write32, kRegHbusTargWdata, 0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kIwmReset[] = {
    {RegOp::Write32, kRegReset, 0x1, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor iwm_wm7260() noexcept {
    return Descriptor{
        .name = "iwm-wm7260",
        .provenance = "hbsd/src/sys/dev/iwm/if_iwmreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorIntel,
        .device_id = kPciDevWm7260,
        .init_sequence = kIwmInit,
        .reset_sequence = kIwmReset,
    };
}

} // namespace pbsd::uda::iwm
