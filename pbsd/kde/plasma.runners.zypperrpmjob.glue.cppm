export module pbsd.kde.plasma.runners.zypperrpmjob.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.runners.zypperrpmjob;

/// Burst 16 wave 2 — Plasma runners.zypperrpmjob ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/runners/plugininstaller/ZypperRpmJob.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::runners::zypperrpmjob::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct ZypperJobStyle {
    const char* package_manager{zypperrpmjob::kPackageManager};
    const char* package_suffix{zypperrpmjob::kRpmExtension};
    unsigned timeout_sec{120};
};

[[nodiscard]] inline Status validate_timeout(unsigned sec) noexcept {
    return sec > 0 && sec <= 600 ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::runners::zypperrpmjob::upstream_path();
}

} // namespace pbsd::kde::plasma::runners::zypperrpmjob::glue
