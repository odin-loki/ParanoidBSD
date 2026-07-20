export module pbsd.kde.plasma.containmentactions_packagestructure;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (plasma_containmentactions_packagestructure.cpp).
/// Upstream: kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_containmentactions_packagestructure.cpp
export namespace pbsd::kde::frameworks::plasma::containmentactions_packagestructure {

inline constexpr const char kPluginId[] = "Plasma/ContainmentActions";
inline constexpr const char kActionsDir[] = "actions";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_containmentactions_packagestructure.cpp";
}

} // namespace pbsd::kde::frameworks::plasma::containmentactions_packagestructure
