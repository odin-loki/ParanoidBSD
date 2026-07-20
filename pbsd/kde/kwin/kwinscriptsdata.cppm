export module pbsd.kde.kwin.kwinscriptsdata;

import pbsd.core;

/// Wave 3 pass 4 — KWin scripts KCM data keys.
/// Upstream: kde/kwin/src/kcms/scripts/kwinscriptsdata.cpp
export namespace pbsd::kde::kwin::kwinscriptsdata {

    inline constexpr const char kScriptsGroup[] = "Plugins";
    inline constexpr const char kScriptIdKey[] = "Id";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/kcms/scripts/kwinscriptsdata.cpp";
}

} // namespace pbsd::kde::kwin::kwinscriptsdata
