export module pbsd.kde.plasma.folder.applauncher.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.folder.applauncher;

/// Burst 15 — Plasma folder.applauncher ↔ Aero glue.
/// Upstream: kde/plasma-desktop/containments/desktop/plugins/folder/applauncher.cpp
export namespace pbsd::kde::plasma::folder::applauncher::glue {

struct LauncherStyle {
    const char* desktop_suffix{folder::applauncher::kDesktopEntrySuffix};
    unsigned max_apps{folder::applauncher::kMaxApps};
    const char* hover_svg{aero::kHoverHighlightSvg};
};

[[nodiscard]] inline LauncherStyle default_style() noexcept {
    return LauncherStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::folder::applauncher::upstream_path();
}

} // namespace pbsd::kde::plasma::folder::applauncher::glue
