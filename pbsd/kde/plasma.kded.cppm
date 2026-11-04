export module pbsd.kde.plasma.kded;

import pbsd.core;

/// Wave 3 pass 4 — KDED modules config group.
/// Upstream: kde/plasma-desktop/kcms/kded/kdedconfigdata.cpp
export namespace pbsd::kde::plasma::kded {

    inline constexpr const char kConfigGroup[] = "Module-kded";
    inline constexpr const char kAutoloadKey[] = "autoload";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/kded/kdedconfigdata.cpp";
}

} // namespace pbsd::kde::plasma::kded
