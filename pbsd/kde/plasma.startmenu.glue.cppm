export module pbsd.kde.plasma.startmenu.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.startmenu;

/// Burst 11 — Start menu popup ↔ Aero blur/frame glue.
/// Upstream: kde/plasma-desktop/containments/panel/startmenu/
export namespace pbsd::kde::plasma::startmenu::glue {

struct StartMenuFrame {
    startmenu::Layout layout{};
    float blur_opacity{0.78f};
    int corner_radius{8};
    const char* frame_svg{startmenu::kFrameSvg};
    const char* background_svg{startmenu::kBackgroundSvg};
};

[[nodiscard]] inline StartMenuFrame default_frame() noexcept {
    StartMenuFrame f{};
    f.layout = startmenu::default_layout();
    f.blur_opacity = f.layout.opacity;
    return f;
}

[[nodiscard]] inline int content_width(const startmenu::Layout& l) noexcept {
    if (l.width <= l.sidebar) {
        return 0;
    }
    return l.width - l.sidebar;
}

[[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams blur_params() noexcept {
    auto p = aero::default_blur();
    p.dialog_opacity = 0.78f;
    p.corner_radius = 8;
    return p;
}

[[nodiscard]] inline Status validate_layout(const startmenu::Layout& l) noexcept {
    if (l.width < 320 || l.height < 400) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return startmenu::kNotesPath;
}

} // namespace pbsd::kde::plasma::startmenu::glue
