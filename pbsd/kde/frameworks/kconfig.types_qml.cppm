export module pbsd.kde.kconfig.types_qml;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (types.cpp).
/// Upstream: kde/frameworks/kconfig/src/qml/types.cpp
export namespace pbsd::kde::frameworks::kconfig::types_qml {

inline constexpr const char kQmlModule[] = "org.kde.kconfig";
inline constexpr const char kTypesUri[] = "org.kde.kconfig.types";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/qml/types.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::types_qml
