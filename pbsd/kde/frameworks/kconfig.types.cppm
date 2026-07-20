export module pbsd.kde.frameworks.kconfig.types;

import pbsd.core;

/// Wave 3 — KConfig QML types registration stub.
/// Upstream: kde/frameworks/kconfig/src/qml/types.cpp
export namespace pbsd::kde::frameworks::kconfig::types {

inline constexpr const char kModuleUri[] = "org.kde.config";
inline constexpr int kModuleMajor = 1;
inline constexpr int kModuleMinor = 0;

[[nodiscard]] inline pbsd::Status register_types() noexcept {
    return pbsd::Status::NotImplemented;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/qml/types.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::types
