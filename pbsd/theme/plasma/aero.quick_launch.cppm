export module pbsd.theme.plasma.aero.quick_launch;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 13 — Aero taskbar quick-launch strip tokens.
export namespace pbsd::theme::plasma::aero::quick_launch {

inline constexpr const char kQuickLaunchSvg[] = "plasma/panel/quick-launch.svg";
inline constexpr int kIconSize{24};
inline constexpr int kMaxSlots{8};
inline constexpr float kGlassOpacity{0.70f};

[[nodiscard]] inline Status validate_slot_count(unsigned count) noexcept {
    return count <= static_cast<unsigned>(kMaxSlots) ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* asset_path() noexcept {
    return ::pbsd::kde::plasma::aero::kQuickLaunchSvg;
}

} // namespace pbsd::theme::plasma::aero::quick_launch
