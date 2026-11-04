export module pbsd.kde.frameworks.plasma.containmentactions;

import pbsd.core;

/// Wave 3 pass 3 — Containment actions package structure.
/// Upstream: kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_containmentactions_packagestructure.cpp
export namespace pbsd::kde::frameworks::plasma::containmentactions {

    inline constexpr const char kPackageStructure[] = "Plasma/ContainmentActions";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_containmentactions_packagestructure.cpp";
}

} // namespace pbsd::kde::frameworks::plasma::containmentactions
