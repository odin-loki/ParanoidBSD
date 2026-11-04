export module pbsd.kde.kwin.wayland.data_device;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 17 — wl_data_device protocol glue.
/// Upstream: kde/kwin/src/wayland/datadevice.cpp
export namespace pbsd::kde::kwin::wayland::data_device {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_DATA_DEVICE",
    "kwin_data_device",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/datadevice.cpp",
};

inline constexpr const char kDeviceInterface[] = "wl_data_device";
inline constexpr const char kSourceInterface[] = "wl_data_source";
inline constexpr int kVersion = 3;

enum class DeviceRequest : unsigned char {
    StartDrag = 0,
    SetSelection = 1,
};

[[nodiscard]] inline bool is_device_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(DeviceRequest::SetSelection);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/datadevice.cpp";
}

} // namespace pbsd::kde::kwin::wayland::data_device
