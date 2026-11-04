export module pbsd.kde.frameworks.kcoreaddons.kaboutdata;

import pbsd.core;

/// Wave 3 — hand port constants (kaboutdata.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/kaboutdata.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kaboutdata {

inline constexpr const char kOrganizationDomain[] = "kde.org";
inline constexpr const char kDesktopFileName[] = "org.kde.kcoreaddons";
inline constexpr const char kVersion[] = "6.0.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/kaboutdata.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kaboutdata
