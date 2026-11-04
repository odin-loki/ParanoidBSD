export module pbsd.kde.plasma.panel;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 5 — Plasma panel layout constants (layout.json).
/// Upstream: kde/plasma-desktop/containments/panel/Panel.qml
export namespace pbsd::kde::plasma::panel {

    inline constexpr int kDefaultHeight{40};
    inline constexpr const char kLayoutPath[] = "plasma/panel/layout.json";
    inline constexpr const char kTaskbarSvg[] = "plasma/panel/taskbar.svg";
    inline constexpr const char kStartButtonSvg[] = "plasma/panel/start-button.svg";
    inline constexpr const char kSystemTraySvg[] = "plasma/panel/system-tray.svg";
    inline constexpr const char kSeparatorSvg[] = "plasma/panel/separator.svg";
    inline constexpr float kDefaultOpacity{0.72f};
    inline constexpr int kDefaultBlurRadius{24};
    enum class Position : unsigned char { Top, Bottom, Left, Right };
    inline constexpr Position kDefaultPosition{Position::Bottom};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/containments/panel/Panel.qml";
}

} // namespace pbsd::kde::plasma::panel
