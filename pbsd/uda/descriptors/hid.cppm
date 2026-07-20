module;
#include <cstdint>

export module pbsd.uda.hid;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/hid/hid.c
export namespace pbsd::uda::hid {

inline constexpr std::uint16_t kUsbClassHid = 0x03;
inline constexpr std::uint8_t kDescReport = 0x22;
inline constexpr std::uint8_t kDescPhysical = 0x23;

inline constexpr RegInsn kHidInit[] = {
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor usb_hid_generic() noexcept {
    return Descriptor{
        .name = "usb-hid",
        .provenance = "hbsd/src/sys/dev/hid/hid.c",
        .device_class = DeviceClass::Input,
        .vendor_id = 0,
        .device_id = 0,
        .init_sequence = kHidInit,
        .reset_sequence = kHidInit,
    };
}

} // namespace pbsd::uda::hid
