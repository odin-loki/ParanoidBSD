module;
#include <cstdint>

export module pbsd.uda.rum;

export import pbsd.uda.schema;

/// PROVENANCE: hbsd/src/sys/dev/usb/wlan/if_rumreg.h
export namespace pbsd::uda::rum {

inline constexpr std::uint32_t kRegMacCtl = 0x100;
inline constexpr std::uint32_t kMacCtlReset = 0x00000001;

inline constexpr std::uint16_t kUsbVendorRalink = 0x148F;
inline constexpr std::uint16_t kUsbDev2573 = 0x2573;

inline constexpr RegInsn kRumInit[] = {
    {RegOp::Write32, kRegMacCtl, kMacCtlReset, 0, 0},
    {RegOp::WaitUs, 0, 50, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor rum_rt2573() noexcept {
    return Descriptor{
        .name = "rum-rt2573",
        .provenance = "hbsd/src/sys/dev/usb/wlan/if_rumreg.h",
        .device_class = DeviceClass::Network,
        .vendor_id = kUsbVendorRalink,
        .device_id = kUsbDev2573,
        .init_sequence = kRumInit,
        .reset_sequence = kRumInit,
    };
}

} // namespace pbsd::uda::rum
