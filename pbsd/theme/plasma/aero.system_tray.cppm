export module pbsd.theme.plasma.aero.system_tray;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 13 — Aero system tray / notification area tokens.
export namespace pbsd::theme::plasma::aero::system_tray {

inline constexpr const char kTraySvg[] = "plasma/panel/system-tray.svg";
inline constexpr const char kTrayItemSvg[] = "plasma/widgets/tray-item.svg";
inline constexpr int kIconSize{16};
inline constexpr int kMaxItems{24};
inline constexpr float kOpacity{0.75f};

[[nodiscard]] inline Status validate_icon_size(int size) noexcept {
    if (size < 12 || size > 32) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_item_count(unsigned count) noexcept {
    return count <= static_cast<unsigned>(kMaxItems) ? Status::Ok : Status::Invalid;
}

} // namespace pbsd::theme::plasma::aero::system_tray
