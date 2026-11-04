export module pbsd.kde.plasma.keyboard.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.keyboard;

/// Burst 13 — Keyboard KCM ↔ Aero settings chrome glue.
export namespace pbsd::kde::plasma::keyboard::glue {

struct KeyboardStyle {
    const char* kcm_id{keyboard::kKcmId};
    const char* settings_svg{aero::kSettingsSvg};
    float panel_opacity{0.75f};
};

[[nodiscard]] inline KeyboardStyle default_style() noexcept {
    return KeyboardStyle{};
}

[[nodiscard]] inline Status validate_kcm_id(const char* id) noexcept {
    return id != nullptr && id[0] != '\0' ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return keyboard::upstream_path();
}

} // namespace pbsd::kde::plasma::keyboard::glue
