export module pbsd.kde.plasma.containmentactions;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Plasma containmentactions constants.
/// Upstream: kde/plasma-framework/containmentactions.cpp
export namespace pbsd::kde::plasma::containmentactions {

inline constexpr const char kPluginPrefix[] = "org.kde.plasma.containmentactions.";
inline constexpr unsigned kMaxActions{16};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-framework/containmentactions.cpp";
}

} // namespace pbsd::kde::plasma::containmentactions
