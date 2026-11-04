export module pbsd.kde.plasma.gamecontroller.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.gamecontroller;

/// Burst 13 — Game controller KCM ↔ Aero settings glue.
export namespace pbsd::kde::plasma::gamecontroller::glue {

struct GameControllerStyle {
    const char* kcm_id{gamecontroller::kKcmId};
    unsigned max_axes{gamecontroller::kMaxAxes};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline GameControllerStyle default_style() noexcept {
    return GameControllerStyle{};
}

[[nodiscard]] inline Status validate_axis_count(unsigned axes) noexcept {
    return axes <= gamecontroller::kMaxAxes ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return gamecontroller::upstream_path();
}

} // namespace pbsd::kde::plasma::gamecontroller::glue
