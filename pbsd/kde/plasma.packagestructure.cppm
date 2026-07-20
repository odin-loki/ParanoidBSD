export module pbsd.kde.plasma.packagestructure;

import pbsd.core;

/// Wave 3 — Plasma generic package structure plugin.
/// Upstream: kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_generic_packagestructure.cpp
export namespace pbsd::kde::plasma::packagestructure {

inline constexpr const char kPluginId[] = "Plasma/Generic";
inline constexpr const char kJsonFile[] = "plasma_generic_packagestructure.json";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_generic_packagestructure.cpp";
}

} // namespace pbsd::kde::plasma::packagestructure
