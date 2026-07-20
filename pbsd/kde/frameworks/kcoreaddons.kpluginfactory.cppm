export module pbsd.kde.frameworks.kcoreaddons.kpluginfactory;

import pbsd.core;

/// Wave 3 — KPluginFactory metadata keys.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/plugin/kpluginfactory.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kpluginfactory {

inline constexpr const char kPluginIdKey[] = "KPlugin";
inline constexpr const char kIdKey[] = "Id";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/plugin/kpluginfactory.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kpluginfactory
