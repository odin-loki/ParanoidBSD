export module pbsd.kde.plasma.calendar.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.calendar;

/// Burst 16 — Plasma calendar ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-desktop/applets/calendar/calendar.cpp
export namespace pbsd::kde::plasma::calendar::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct CalendarStyle {
    const char* applet_id{calendar::kAppletId};
    unsigned week_rows{calendar::kWeekRows};
    const char* hover_svg{aero::kHoverHighlightSvg};
};

[[nodiscard]] inline CalendarStyle default_style() noexcept {
    return CalendarStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::calendar::upstream_path();
}

} // namespace pbsd::kde::plasma::calendar::glue
