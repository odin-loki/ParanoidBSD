export module pbsd.kde.frameworks.kcoreaddons.kcoreaddons;

import pbsd.core;

/// Wave 3 — KCoreAddons version constants (from kcoreaddons.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/kcoreaddons.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::version {

inline constexpr const char kVersionString[] = "6.0.0-pbsd";
inline constexpr unsigned kVersionMajor = 6;
inline constexpr unsigned kVersionMinor = 0;
inline constexpr unsigned kVersionPatch = 0;

[[nodiscard]] inline unsigned version() noexcept {
    return (kVersionMajor << 16) | (kVersionMinor << 8) | kVersionPatch;
}

[[nodiscard]] inline const char* version_string() noexcept { return kVersionString; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/kcoreaddons.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::version
