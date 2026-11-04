export module pbsd.kde.plasma.keyboard;

import pbsd.core;

/// Wave 3 pass 3 — Keyboard KCM module id.
/// Upstream: kde/plasma-desktop/kcms/keyboard/kcmmain.cpp
export namespace pbsd::kde::plasma::keyboard {

    inline constexpr const char kKcmId[] = "kcm_keyboard";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/keyboard/kcmmain.cpp";
}

} // namespace pbsd::kde::plasma::keyboard
