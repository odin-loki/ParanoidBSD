export module pbsd.kde.kwin.wayland.primary_selection;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 18 — zwp_primary_selection_device_manager_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/primaryselection_v1.cpp
export namespace pbsd::kde::kwin::wayland::primary_selection {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_PRIMARY_SELECTION",
    "kwin_primary_selection",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/primaryselection_v1.cpp",
};

inline constexpr const char kManagerInterface[] = "zwp_primary_selection_device_manager_v1";
inline constexpr const char kDeviceInterface[] = "zwp_primary_selection_device_v1";
inline constexpr const char kSourceInterface[] = "zwp_primary_selection_source_v1";
inline constexpr int kVersion = 1;

enum class DeviceRequest : unsigned char {
    Destroy = 0,
    SetSelection = 1,
};

[[nodiscard]] inline bool is_device_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(DeviceRequest::SetSelection);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/primaryselection_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::primary_selection
