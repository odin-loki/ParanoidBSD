export module pbsd.kde.frameworks.kwidgets.kseparator;

import pbsd.core;

/// Wave 3 pass 5 — KSeparator orientation constants.
/// Upstream: kde/frameworks/kwidgetsaddons/src/kseparator.cpp
export namespace pbsd::kde::frameworks::kwidgets::kseparator {

    enum class Orientation : unsigned char { Horizontal, Vertical };
    inline constexpr int kDefaultLineWidth = 1;
    inline constexpr int kDefaultMargin = 4;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwidgetsaddons/src/kseparator.cpp";
}

} // namespace pbsd::kde::frameworks::kwidgets::kseparator
