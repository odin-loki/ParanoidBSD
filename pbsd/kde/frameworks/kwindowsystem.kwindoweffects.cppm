export module pbsd.kde.frameworks.kwindowsystem.kwindoweffects;

import pbsd.core;

/// Wave 3 pass 4 — KWindowEffects capability flags.
/// Upstream: kde/frameworks/kwindowsystem/src/kwindoweffects.cpp
export namespace pbsd::kde::frameworks::kwindowsystem::kwindoweffects {

    inline constexpr unsigned kBlurBehind = 0x01;
    inline constexpr unsigned kSlide = 0x02;
    inline constexpr unsigned kPresentWindows = 0x04;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwindowsystem/src/kwindoweffects.cpp";
}

} // namespace pbsd::kde::frameworks::kwindowsystem::kwindoweffects
