export module pbsd.kde.kwin.wayland.tearing_control;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 18 — wp_tearing_control_manager_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/tearingcontrol_v1.cpp
export namespace pbsd::kde::kwin::wayland::tearing_control {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_TEARING_CONTROL",
    "kwin_tearing_control",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/tearingcontrol_v1.cpp",
};

inline constexpr const char kManagerInterface[] = "wp_tearing_control_manager_v1";
inline constexpr const char kControlInterface[] = "wp_tearing_control_v1";
inline constexpr int kVersion = 1;

enum class Hint : unsigned char {
    Never = 0,
    WhenPossible = 1,
    Always = 2,
};

[[nodiscard]] inline Status validate_hint(unsigned hint) noexcept {
    return hint <= static_cast<unsigned>(Hint::Always) ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/tearingcontrol_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::tearing_control
