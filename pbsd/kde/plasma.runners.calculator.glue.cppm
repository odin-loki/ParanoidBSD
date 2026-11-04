export module pbsd.kde.plasma.runners.calculator.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.runners.calculator;

/// Burst 17 — Plasma calculator runner ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-workspace/runners/calculator/calculator.cpp
export namespace pbsd::kde::plasma::runners::calculator::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct CalculatorStyle {
    const char* runner_id{calculator::kRunnerId};
    const char* trigger{calculator::kTrigger};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline CalculatorStyle default_style() noexcept {
    return CalculatorStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::runners::calculator::upstream_path();
}

} // namespace pbsd::kde::plasma::runners::calculator::glue
