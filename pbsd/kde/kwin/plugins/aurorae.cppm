export module pbsd.kde.kwin.plugins.aurorae;

import pbsd.core;

/// Wave 3 pass 4 — Aurorae decoration package id.
/// Upstream: kde/kwin/src/plugins/kpackage/aurorae/aurorae.cpp
export namespace pbsd::kde::kwin::plugins::aurorae {

    inline constexpr const char kPackageId[] = "org.kde.kwin.aurorae";
    inline constexpr const char kThemeKey[] = "theme";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/kpackage/aurorae/aurorae.cpp";
}

} // namespace pbsd::kde::kwin::plugins::aurorae
