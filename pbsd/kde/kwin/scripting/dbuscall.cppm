export module pbsd.kde.dbuscall;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (dbuscall.cpp).
/// Upstream: kde/kwin/src/scripting/dbuscall.cpp
export namespace pbsd::kde::dbuscall {

inline constexpr const char kScriptingService[] = "org.kde.KWin";
inline constexpr const char kScriptingPath[] = "/Scripting";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/scripting/dbuscall.cpp";
}

} // namespace pbsd::kde::dbuscall
