export module pbsd.kde.plasma.runners.abstractjob.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.runners.abstractjob;

/// Burst 16 wave 2 — Plasma runners.abstractjob ↔ Aero glue.
/// Upstream: kde/plasma-desktop/kcms/runners/plugininstaller/AbstractJob.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::runners::abstractjob::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct RunnerJobStyle {
    const char* pending{abstractjob::kJobStatePending};
    const char* finished{abstractjob::kJobStateFinished};
};

[[nodiscard]] inline RunnerJobStyle default_style() noexcept { return RunnerJobStyle{}; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::runners::abstractjob::upstream_path();
}

} // namespace pbsd::kde::plasma::runners::abstractjob::glue
