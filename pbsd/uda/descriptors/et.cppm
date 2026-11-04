module;
#include <cstdint>

export module pbsd.uda.et;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/et/if_etreg.h
export namespace pbsd::uda::et {

inline constexpr std::uint32_t kRegIntrMask = 0x0000;
inline constexpr std::uint32_t kIntrDisable = 0xFFFFFFFFu;
inline constexpr std::uint16_t kPciVendorAgere = 0x11C1;
inline constexpr std::uint16_t kPciDev5801 = 0x5801;

inline constexpr RegInsn kEtInit[] = {
    {RegOp::Write32, kRegIntrMask, kIntrDisable, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor et_1310() noexcept {
    return Descriptor{
        .name = "et-1310",
        .provenance = "hbsd/src/sys/dev/et/if_etreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorAgere,
        .device_id = kPciDev5801,
        .init_sequence = kEtInit,
        .reset_sequence = kEtInit,
    };
}

} // namespace pbsd::uda::et
