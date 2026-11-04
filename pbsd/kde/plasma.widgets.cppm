export module pbsd.kde.plasma.widgets;

import pbsd.core;

/// Wave 3 pass 6 — Plasma widget SVG asset ids (Aero controls).
/// Upstream: pbsd/theme/plasma/theme-colors.json
export namespace pbsd::kde::plasma::widgets {

    enum class Asset : unsigned char {
        Background, Button, Frame, Notification, Slider, Scrollbar, Tab, Checkbox,
        Radiobutton, Tasks, Tooltip, MenuItem, Progress, Spinner, HoverHighlight,
        Combobox, Listbox, Treeview, Header, Footer, Badge, Pin, Volume, Battery,
        Wifi, Bluetooth, Dropdown, Editbox, Groupbox, Splitter
    };
    inline constexpr const char kTabSvg[] = "plasma/widgets/tab.svg";
    inline constexpr const char kCheckboxSvg[] = "plasma/widgets/checkbox.svg";
    inline constexpr const char kRadiobuttonSvg[] = "plasma/widgets/radiobutton.svg";
    inline constexpr const char kMenuItemSvg[] = "plasma/widgets/menuitem.svg";
    inline constexpr const char kProgressSvg[] = "plasma/widgets/progress.svg";
    inline constexpr const char kSpinnerSvg[] = "plasma/widgets/spinner.svg";
    inline constexpr const char kHoverHighlightSvg[] = "plasma/widgets/hover-highlight.svg";
    inline constexpr const char kNotificationSvg[] = "plasma/widgets/notification.svg";
    inline constexpr int kDefaultIconSize{22};
    inline constexpr int kDefaultFrameRadius{4};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "pbsd/theme/plasma/theme-colors.json";
}

} // namespace pbsd::kde::plasma::widgets
