export module pbsd.kde.plasma.runners.bookmarks.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.runners.bookmarks;

/// Burst 18 — Plasma bookmarks runner ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-workspace/runners/bookmarks/bookmarks.cpp
export namespace pbsd::kde::plasma::runners::bookmarks::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct BookmarksStyle {
    const char* runner_id{bookmarks::kRunnerId};
    const char* trigger{bookmarks::kTrigger};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline BookmarksStyle default_style() noexcept {
    return BookmarksStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::runners::bookmarks::upstream_path();
}

} // namespace pbsd::kde::plasma::runners::bookmarks::glue
