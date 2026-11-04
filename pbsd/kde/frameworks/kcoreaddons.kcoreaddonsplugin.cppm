export module pbsd.kde.frameworks.kcoreaddons.kcoreaddonsplugin;

import pbsd.core;

/// Wave 3 — KCoreAddons QML plugin registration.
/// Upstream: kde/frameworks/kcoreaddons/src/qml/kcoreaddonsplugin.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kcoreaddonsplugin {

inline constexpr const char kPluginId[] = "kcoreaddonsplugin";
inline constexpr const char kQmlModule[] = "org.kde.kcoreaddons";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/qml/kcoreaddonsplugin.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kcoreaddonsplugin
