export module pbsd.kde.frameworks.kwidgets.lineedit;

import pbsd.core;

/// Wave 3 pass 5 — KLineEdit clear-button and frame constants.
/// Upstream: kde/frameworks/kwidgetsaddons/src/klineedit.cpp
export namespace pbsd::kde::frameworks::kwidgets::lineedit {

    inline constexpr int kClearButtonSize = 16;
    inline constexpr int kFrameMargin = 4;
    inline constexpr unsigned kMaxLengthDefault = 32767;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwidgetsaddons/src/klineedit.cpp";
}

} // namespace pbsd::kde::frameworks::kwidgets::lineedit
