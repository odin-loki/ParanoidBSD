module;
#include <cstdint>

export module pbsd.uda.acpi_video;

export import pbsd.uda.schema;
export import pbsd.uda.interp;

/// PROVENANCE: hbsd/src/sys/dev/acpica/acpi_video.c — ACPI video notify + brightness.
export namespace pbsd::uda::acpi_video {

inline constexpr std::uint32_t kNotifySwitched   = 0x80;
inline constexpr std::uint32_t kNotifyReprobe    = 0x81;
inline constexpr std::uint32_t kNotifyCycleOut   = 0x82;
inline constexpr std::uint32_t kNotifyNextOut     = 0x83;
inline constexpr std::uint32_t kNotifyPrevOut     = 0x84;
inline constexpr std::uint32_t kNotifyCycleBrn    = 0x85;
inline constexpr std::uint32_t kNotifyIncBrn      = 0x86;
inline constexpr std::uint32_t kNotifyDecBrn      = 0x86 + 1;
inline constexpr std::uint32_t kNotifyZeroBrn     = 0x88;
inline constexpr std::uint32_t kNotifyDispOff     = 0x89;

inline constexpr std::uint32_t kRegNotifyAck = 0;
inline constexpr std::uint32_t kRegBrightness  = 4;
inline constexpr std::uint32_t kRegOutputMask = 8;

inline constexpr std::uint32_t kAckReady = 0x1;
inline constexpr std::uint32_t kBrnMax   = 100;

inline constexpr RegInsn kAcpiVideoInit[] = {
    {RegOp::Write32, kRegNotifyAck, kAckReady, 0, 0},
    {RegOp::Write32, kRegOutputMask, 0x1, 0, 0},
    {RegOp::Write32, kRegBrightness, kBrnMax / 2, 0, 0},
    {RegOp::CheckEq, kRegNotifyAck, kAckReady, kAckReady, 1000},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAcpiVideoReset[] = {
    {RegOp::Write32, kRegBrightness, 0x0, 0, 0},
    {RegOp::Write32, kRegOutputMask, 0x0, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

inline constexpr RegInsn kAcpiVideoIncBrn[] = {
    {RegOp::Write32, kRegNotifyAck, kNotifyIncBrn, 0, 0},
    {RegOp::Done, 0, 0, 0, 0},
};

[[nodiscard]] inline constexpr Descriptor acpi_video_backlight() noexcept {
    return Descriptor{
        .name = "acpi-video-backlight",
        .provenance = "hbsd/src/sys/dev/acpica/acpi_video.c",
        .device_class = DeviceClass::Display,
        .vendor_id = 0x0000,
        .device_id = 0x0000,
        .init_sequence = kAcpiVideoInit,
        .reset_sequence = kAcpiVideoReset,
    };
}

[[nodiscard]] constexpr bool is_brightness_notify(std::uint32_t code) noexcept {
    return code == kNotifyCycleBrn || code == kNotifyIncBrn || code == kNotifyDecBrn
        || code == kNotifyZeroBrn;
}

[[nodiscard]] constexpr std::uint32_t clamp_brightness(int level) noexcept {
    if (level < 0) {
        return 0;
    }
    if (level > static_cast<int>(kBrnMax)) {
        return kBrnMax;
    }
    return static_cast<std::uint32_t>(level);
}

} // namespace pbsd::uda::acpi_video
