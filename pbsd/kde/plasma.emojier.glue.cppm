export module pbsd.kde.plasma.emojier.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.emojier;

/// Burst 14 — Plasma emojier ↔ Aero glue.
/// Upstream: kde/plasma-desktop/applets/emojier/plugin/emojier.cpp
export namespace pbsd::kde::plasma::emojier::glue {

struct EmojierStyle {
    const char* applet_id{"org.kde.plasma.emojier"};
    unsigned max_recent{32};
};

[[nodiscard]] inline EmojierStyle default_style() noexcept {
    return EmojierStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return emojier::upstream_path();
}

} // namespace pbsd::kde::plasma::emojier::glue
