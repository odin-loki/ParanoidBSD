module;
#include <cstdint>

export module pbsd.uda.age;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/age/if_agereg.h
export namespace pbsd::uda::age {

inline constexpr std::uint32_t kRegMaster = 0x0000;
inline constexpr std::uint32_t kMasterReset = 0x00000001;
inline constexpr std::uint16_t kPciVendorAmd = 0x1022;
inline constexpr std::uint16_t kPciDev1700 = 0x1700;

inline constexpr RegInsn kAgeInit[] = {
    {RegOp::Write32, kRegMaster, kMasterReset, 0, 0},
    {RegOp::WaitUs, 0, 100, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor age_1700() noexcept {
    return Descriptor{
        .name = "age-1700",
        .provenance = "hbsd/src/sys/dev/age/if_agereg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kPciVendorAmd,
        .device_id = kPciDev1700,
        .init_sequence = kAgeInit,
        .reset_sequence = kAgeInit,
    };
}

} // namespace pbsd::uda::age
