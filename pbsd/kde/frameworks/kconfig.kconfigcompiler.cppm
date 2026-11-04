export module pbsd.kde.frameworks.kconfig.kconfigcompiler;

import pbsd.core;

/// Wave 3 — hand port constants (kconfigcompiler.cpp).
/// Upstream: kde/frameworks/kconfig/src/kconfig_compiler/kconfigcompiler.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfigcompiler {

inline constexpr const char kKcfgSuffix[] = ".kcfg";
inline constexpr const char kClassSuffix[] = "Skeleton";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/kconfig_compiler/kconfigcompiler.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfigcompiler
