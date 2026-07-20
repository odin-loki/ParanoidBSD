export module pbsd.kde.kwin.wayland.tablet_v2;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 17 — zwp_tablet_manager_v2 protocol glue.
/// Upstream: kde/kwin/src/wayland/tablet_v2.cpp
export namespace pbsd::kde::kwin::wayland::tablet_v2 {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_TABLET_V2",
    "kwin_tablet_v2",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/tablet_v2.cpp",
};

inline constexpr const char kManagerInterface[] = "zwp_tablet_manager_v2";
inline constexpr const char kTabletInterface[] = "zwp_tablet_v2";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    GetTabletSeat = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::GetTabletSeat);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/tablet_v2.cpp";
}

} // namespace pbsd::kde::kwin::wayland::tablet_v2
