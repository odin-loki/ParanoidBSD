export module pbsd.theme.plasma.aero.screen_lock;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Aero screen lock visual tokens.
export namespace pbsd::theme::plasma::aero::screen_lock {

struct ScreenLockTokens {
    unsigned blur_radius_px{32};
    float overlay_opacity{0.55f};
    const char* lock_svg{"plasma/panel/lock.svg"};
};

[[nodiscard]] inline ScreenLockTokens default_tokens() noexcept {
    return ScreenLockTokens{};
}

} // namespace pbsd::theme::plasma::aero::screen_lock
