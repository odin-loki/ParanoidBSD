export module pbsd.kde.frameworks.kconfig.inifile;

import pbsd.core;

/// Wave 3 pass 5 — KConfig INI file group/key limits.
/// Upstream: kde/frameworks/kconfig/src/kconfigini.cpp
export namespace pbsd::kde::frameworks::kconfig::inifile {

    inline constexpr unsigned kMaxGroupLen = 256;
    inline constexpr unsigned kMaxKeyLen = 256;
    inline constexpr unsigned kMaxValueLen = 4096;
    inline constexpr const char kDefaultGroup[] = "General";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/kconfigini.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::inifile
