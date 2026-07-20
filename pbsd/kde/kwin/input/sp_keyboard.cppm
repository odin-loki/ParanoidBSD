export module pbsd.kde.sp_keyboard;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (sp_keyboard.cpp).
/// Upstream: kde/kwin/src/input/sp_keyboard.cpp
export namespace pbsd::kde::sp_keyboard {

inline constexpr unsigned kMaxKeys = 256;
inline constexpr unsigned kRepeatDelayMs = 600;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/input/sp_keyboard.cpp";
}

} // namespace pbsd::kde::sp_keyboard
