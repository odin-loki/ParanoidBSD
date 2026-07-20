export module pbsd.kde.plasma.dialogs.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.dialogs;

/// Burst 14 — Plasma dialogs ↔ Aero glue.
/// Upstream: kde/plasma-desktop/containments/panel/dialogs/Dialog.qml
export namespace pbsd::kde::plasma::dialogs::glue {

struct DialogStyle {
    float opacity{aero::default_blur().dialog_opacity};
    int blur_radius{aero::default_blur().blur_radius};
    const char* frame_svg{aero::kMessageBoxSvg};
};

[[nodiscard]] inline DialogStyle default_style() noexcept {
    return DialogStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return dialogs::upstream_path();
}

} // namespace pbsd::kde::plasma::dialogs::glue
