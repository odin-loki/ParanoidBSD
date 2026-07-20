export module pbsd.kde.plasma.runners.calculator;

import pbsd.core;

/// Burst 17 — Plasma calculator runner constants.
/// Upstream: kde/plasma-workspace/runners/calculator/calculator.cpp
export namespace pbsd::kde::plasma::runners::calculator {

inline constexpr const char kRunnerId[] = "calculator";
inline constexpr const char kTrigger[] = "=";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/runners/calculator/calculator.cpp";
}

} // namespace pbsd::kde::plasma::runners::calculator
