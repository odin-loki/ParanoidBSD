module;
#include <cstdint>

export module pbsd.uda.zyd;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/usb/wlan/if_zydreg.h
export namespace pbsd::uda::zyd {

inline constexpr std::uint32_t kRegMacCtl = 0x0100;
inline constexpr std::uint32_t kMacCtlReset = 0x00000001;

inline constexpr std::uint16_t kUsbVendorZydas = 0x0586;
inline constexpr std::uint16_t kUsbDev1211 = 0x3410;

inline constexpr RegInsn kZydInit[] = {
    {RegOp::Write32, kRegMacCtl, kMacCtlReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor zyd_zd1211() noexcept {
    return Descriptor{
        .name = "zyd-zd1211",
        .provenance = "hbsd/src/sys/dev/usb/wlan/if_zydreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kUsbVendorZydas,
        .device_id = kUsbDev1211,
        .init_sequence = kZydInit,
        .reset_sequence = kZydInit,
    };
}

} // namespace pbsd::uda::zyd
