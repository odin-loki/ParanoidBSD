export module pbsd.kde.frameworks.kwidgets.kswitch;

import pbsd.core;

/// Wave 3 pass 5 — KSwitch toggle widget constants.
/// Upstream: kde/frameworks/kwidgetsaddons/src/kswitch.cpp
export namespace pbsd::kde::frameworks::kwidgets::kswitch {

    inline constexpr int kDefaultWidth = 48;
    inline constexpr int kDefaultHeight = 24;
    inline constexpr float kAnimationDurationMs = 150.0f;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwidgetsaddons/src/kswitch.cpp";
}

} // namespace pbsd::kde::frameworks::kwidgets::kswitch
