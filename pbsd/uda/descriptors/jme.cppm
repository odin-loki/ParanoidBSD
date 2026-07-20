module;
#include <cstdint>

export module pbsd.uda.jme;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/jme/if_jmereg.h
export namespace pbsd::uda::jme {

inline constexpr std::uint32_t kRegSmi = 0x0050;
inline constexpr std::uint32_t kGhcReset = 0x40000000;

inline constexpr std::uint16_t kPciVendorJmicron = 0x197B;
inline constexpr std::uint16_t kPciDevJmc250 = 0x0250;

inline constexpr RegInsn kJmeInit[] = {
    {RegOp::Write32, kRegSmi, kGhcReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kJmeReset[] = {
    {RegOp::Write32, kRegSmi, kGhcReset, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor jme_jmc250() noexcept {
    return Descriptor{
        .name = "jme-jmc250",
        .provenance = "hbsd/src/sys/dev/jme/if_jmereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorJmicron,
        .device_id = kPciDevJmc250,
        .init_sequence = kJmeInit,
        .reset_sequence = kJmeReset,
    };
}

} // namespace pbsd::uda::jme
