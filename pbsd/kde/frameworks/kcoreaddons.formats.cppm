export module pbsd.kde.frameworks.kcoreaddons.formats;

import pbsd.core;

/// Wave 3 — KCoreAddons QML format helpers.
/// Upstream: kde/frameworks/kcoreaddons/src/qml/formats.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::formats {

inline constexpr const char kQmlModule[] = "org.kde.kcoreaddons";
inline constexpr const char kFormatsUri[] = "org.kde.kcoreaddons.formats";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/qml/formats.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::formats
