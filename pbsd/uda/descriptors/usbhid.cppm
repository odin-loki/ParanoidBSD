module;
#include <cstdint>

export module pbsd.uda.usbhid;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: USB HID 1.11 + hbsd usbhid class requests.
export namespace pbsd::uda::usbhid {

inline constexpr std::uint8_t kReqSetProtocol = 0x0B;
inline constexpr std::uint8_t kReqSetIdle     = 0x0A;
inline constexpr std::uint8_t kReqGetReport  = 0x01;

inline constexpr std::uint8_t kProtocolBoot  = 0x00;
inline constexpr std::uint8_t kProtocolReport = 0x01;

inline constexpr std::uint8_t kReportInput  = 0x01;
inline constexpr std::uint8_t kReportOutput = 0x02;
inline constexpr std::uint8_t kReportFeature = 0x03;

inline constexpr RegInsn kUsbHidInit[] = {
    {RegOp::Write8, 0x00, kReqSetProtocol, 0, 0},
    {RegOp::Write8, 0x01, kProtocolBoot, 0, 0},
    {RegOp::Write8, 0x02, kReqSetIdle, 0, 0},
    {RegOp::Write8, 0x03, 0x00, 0, 0},
    {RegOp::Write8, 0x04, kReqGetReport, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kUsbHidReset[] = {
    {RegOp::Write8, 0x00, 0x00, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor usb_hid_boot() noexcept {
    return Descriptor{
        .name = "usb-hid-boot",
        .provenance = "USB-HID-1.11",
        .device_class = DeviceClass::Input,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kUsbHidInit,
        .reset_sequence = kUsbHidReset,
    };
}

[[nodiscard]] inline bool probe_control(SoftMmio const& mem) noexcept {
    return mem.read8(0x00) == kReqSetProtocol || mem.read8(0x00) == 0;
}

} // namespace pbsd::uda::usbhid
