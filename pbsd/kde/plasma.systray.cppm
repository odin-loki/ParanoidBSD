export module pbsd.kde.plasma.systray;

import pbsd.core;

/// Wave 3 pass 5 — System tray applet constants.
/// Upstream: kde/plasma-desktop/applets/systemtray/package/contents/config/main.xml
export namespace pbsd::kde::plasma::systray {

    inline constexpr const char kAppletId[] = "org.kde.plasma.systemtray";
    inline constexpr const char kTraySvg[] = "plasma/panel/system-tray.svg";
    inline constexpr unsigned kMaxItems{32};
    inline constexpr int kIconSize{22};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/applets/systemtray/package/contents/config/main.xml";
}

} // namespace pbsd::kde::plasma::systray
