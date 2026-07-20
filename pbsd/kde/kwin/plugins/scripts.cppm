export module pbsd.kde.kwin.plugins.scripts;

import pbsd.core;

/// Wave 3 pass 4 — KWin scripts kpackage structure.
/// Upstream: kde/kwin/src/plugins/kpackage/scripts/scripts.cpp
export namespace pbsd::kde::kwin::plugins::scripts {

    inline constexpr const char kPackageStructure[] = "KWin/Script";
    inline constexpr const char kScriptSuffix[] = ".js";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/kpackage/scripts/scripts.cpp";
}

} // namespace pbsd::kde::kwin::plugins::scripts
