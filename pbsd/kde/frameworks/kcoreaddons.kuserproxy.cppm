export module pbsd.kde.frameworks.kcoreaddons.kuserproxy;

import pbsd.core;

/// Wave 3 — KUser QML proxy property keys.
/// Upstream: kde/frameworks/kcoreaddons/src/qml/kuserproxy.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kuserproxy {

inline constexpr const char kUidKey[] = "uid";
inline constexpr const char kLoginNameKey[] = "loginName";
inline constexpr const char kHomeDirKey[] = "homeDir";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/qml/kuserproxy.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kuserproxy
