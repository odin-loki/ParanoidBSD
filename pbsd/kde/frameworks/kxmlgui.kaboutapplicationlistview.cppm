export module pbsd.kde.frameworks.kxmlgui.kaboutapplicationlistview;

import pbsd.core;

/// Wave 3 pass 4 — About dialog list view keys.
/// Upstream: kde/frameworks/kxmlgui/src/kaboutapplicationlistview_p.cpp
export namespace pbsd::kde::frameworks::kxmlgui::kaboutapplicationlistview {

    inline constexpr const char kAppNameRole[] = "appName";
    inline constexpr const char kVersionRole[] = "version";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kxmlgui/src/kaboutapplicationlistview_p.cpp";
}

} // namespace pbsd::kde::frameworks::kxmlgui::kaboutapplicationlistview
