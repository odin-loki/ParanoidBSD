export module pbsd.kde.plasma.wheelinterceptor.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.wheelinterceptor;

/// Burst 15 — Plasma wheelinterceptor ↔ Aero glue.
/// Upstream: kde/plasma-desktop/containments/desktop/plugins/folder/wheelinterceptor.cpp
export namespace pbsd::kde::plasma::wheelinterceptor::glue {

struct WheelStyle {
    int max_delta{wheelinterceptor::kMaxDelta};
    int min_delta{wheelinterceptor::kMinDelta};
    const char* hover_svg{aero::kHoverHighlightSvg};
};

[[nodiscard]] inline WheelStyle default_style() noexcept {
    return WheelStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::wheelinterceptor::upstream_path();
}

} // namespace pbsd::kde::plasma::wheelinterceptor::glue
