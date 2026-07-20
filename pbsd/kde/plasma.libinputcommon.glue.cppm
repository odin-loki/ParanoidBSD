export module pbsd.kde.plasma.libinputcommon.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.libinputcommon;

/// Burst 14 — Plasma libinputcommon ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/kcm_mouse/libinputcommon.cpp
export namespace pbsd::kde::plasma::libinputcommon::glue {

struct InputStyle {
    const char* driver{"libinput"};
    unsigned scroll_factor{120};
};

[[nodiscard]] inline InputStyle default_style() noexcept {
    return InputStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return libinputcommon::upstream_path();
}

} // namespace pbsd::kde::plasma::libinputcommon::glue
