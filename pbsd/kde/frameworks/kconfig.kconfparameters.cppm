module;

#include <cstring>

export module pbsd.kde.frameworks.kconfig.kconfparameters;

import pbsd.core;

/// Wave 3 — KConfig compiler .kcfgc parameter keys.
/// Upstream: kde/frameworks/kconfig/src/kconfig_compiler/KConfigParameters.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfparameters {

inline constexpr const char kCodegenSuffix[] = ".kcfgc";
inline constexpr const char kDefaultInherits[] = "KConfigSkeleton";
inline constexpr unsigned kSuffixLen = 6;

[[nodiscard]] inline bool has_kcfgc_suffix(const char* path) noexcept {
    if (path == nullptr) {
        return false;
    }
    const unsigned len = static_cast<unsigned>(std::strlen(path));
    if (len < kSuffixLen) {
        return false;
    }
    return std::strcmp(path + len - kSuffixLen, kCodegenSuffix) == 0;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/kconfig_compiler/KConfigParameters.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfparameters
