export module pbsd.kde.frameworks.kconfig.kconfigpropertymap;

import pbsd.core;

/// Wave 3 — KConfigPropertyMap QML bridge keys.
/// Upstream: kde/frameworks/kconfig/src/qml/kconfigpropertymap.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfigpropertymap {

inline constexpr const char kQmlModule[] = "org.kde.kconfig";
inline constexpr const char kTypeName[] = "KConfigPropertyMap";
inline constexpr unsigned kMaxProperties = 128;
inline constexpr unsigned kMaxKeyLen = 128;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/qml/kconfigpropertymap.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfigpropertymap
