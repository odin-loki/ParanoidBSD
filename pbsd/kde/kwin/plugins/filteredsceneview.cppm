export module pbsd.kde.kwin.plugins.filteredsceneview;

import pbsd.core;

/// Wave 3 pass 4 — Screencast filtered scene view.
/// Upstream: kde/kwin/src/plugins/screencast/filteredsceneview.cpp
export namespace pbsd::kde::kwin::plugins::filteredsceneview {

    inline constexpr const char kEffectId[] = "screencast";
    inline constexpr unsigned kMaxFilters = 8;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/screencast/filteredsceneview.cpp";
}

} // namespace pbsd::kde::kwin::plugins::filteredsceneview
