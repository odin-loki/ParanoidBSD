export module pbsd.kde.plasma.qtquicksettings;

import pbsd.core;

/// Wave 3 pass 4 — Qt Quick settings KCM.
/// Upstream: kde/plasma-desktop/kcms/qtquicksettings/kcmqtquicksettings.cpp
export namespace pbsd::kde::plasma::qtquicksettings {

    inline constexpr const char kKcmId[] = "kcm_qtquicksettings";
    inline constexpr const char kRenderLoopKey[] = "renderLoop";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/qtquicksettings/kcmqtquicksettings.cpp";
}

} // namespace pbsd::kde::plasma::qtquicksettings
