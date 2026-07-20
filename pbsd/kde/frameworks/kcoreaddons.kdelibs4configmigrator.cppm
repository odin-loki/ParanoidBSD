export module pbsd.kde.frameworks.kcoreaddons.kdelibs4configmigrator;

import pbsd.core;

/// Wave 3 — hand port constants (kdelibs4configmigrator.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/kdelibs4configmigrator.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kdelibs4configmigrator {

inline constexpr const char kKde4ConfigDir[] = ".kde4/share/config";
inline constexpr const char kKde5ConfigDir[] = ".config";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/kdelibs4configmigrator.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kdelibs4configmigrator
