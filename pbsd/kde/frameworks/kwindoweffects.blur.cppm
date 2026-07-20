export module pbsd.kde.frameworks.kwindowsystem.kwindoweffects.blur;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.frameworks.kwindowsystem.kwindoweffects;

/// Wave 3 pass 7 — KWindowEffects blur-behind ↔ Aero BlurParams glue.
/// Upstream: kde/frameworks/kwindowsystem/src/kwindoweffects.cpp
export namespace pbsd::kde::frameworks::kwindowsystem::kwindoweffects::blur {

[[nodiscard]] inline bool supports_blur_behind(unsigned capabilities) noexcept {
    return (capabilities & kwindoweffects::kBlurBehind) != 0;
}

[[nodiscard]] inline plasma::aero::BlurParams params_for_window(unsigned capabilities) noexcept {
    auto p = plasma::aero::default_blur();
    if (supports_blur_behind(capabilities)) {
        p.dialog_opacity = 0.78f;
    }
    return p;
}

[[nodiscard]] inline plasma::aero::BlurParams params_for_panel(unsigned capabilities) noexcept {
    auto p = plasma::aero::default_blur();
    if (supports_blur_behind(capabilities)) {
        p.panel_opacity = 0.72f;
    }
    return p;
}

[[nodiscard]] inline Status enable_blur(unsigned& capabilities) noexcept {
    capabilities |= kwindoweffects::kBlurBehind;
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return kwindoweffects::upstream_path();
}

} // namespace pbsd::kde::frameworks::kwindowsystem::kwindoweffects::blur
