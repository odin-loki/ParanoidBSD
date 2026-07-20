export module pbsd.kde.plasma.systray.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.panel;
import pbsd.kde.plasma.systray;

/// Burst 12 — System tray applet ↔ Aero panel asset glue.
/// Upstream: kde/plasma-desktop/applets/systemtray/
export namespace pbsd::kde::plasma::systray::glue {

struct TrayLayout {
    unsigned max_items{systray::kMaxItems};
    int icon_size{systray::kIconSize};
    int panel_height{panel::kDefaultHeight};
    const char* tray_svg{systray::kTraySvg};
    const char* notification_svg{aero::kNotificationSvg};
};

[[nodiscard]] inline TrayLayout default_layout() noexcept {
    return TrayLayout{};
}

[[nodiscard]] inline int tray_width(unsigned item_count, int icon_size) noexcept {
    if (item_count == 0 || icon_size <= 0) {
        return 0;
    }
    const unsigned capped = item_count > systray::kMaxItems ? systray::kMaxItems : item_count;
    return static_cast<int>(capped) * (icon_size + 4) + 8;
}

[[nodiscard]] inline Status validate_icon_size(int size) noexcept {
    if (size < 16 || size > 48) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return systray::upstream_path();
}

} // namespace pbsd::kde::plasma::systray::glue
