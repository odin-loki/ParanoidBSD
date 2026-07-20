export module pbsd.kde.plasma.globalpaths.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.globalpaths;

/// Burst 14 — Plasma globalpaths ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/desktoppaths/globalpaths.cpp
export namespace pbsd::kde::plasma::globalpaths::glue {

struct PathsStyle {
    const char* desktop{globalpaths::kDesktopLocation};
    const char* documents{globalpaths::kDocumentsLocation};
    const char* pictures{globalpaths::kPicturesLocation};
};

[[nodiscard]] inline PathsStyle default_style() noexcept {
    return PathsStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return globalpaths::upstream_path();
}

} // namespace pbsd::kde::plasma::globalpaths::glue
