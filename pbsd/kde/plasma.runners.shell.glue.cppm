export module pbsd.kde.plasma.runners.shell.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.runners.abstractjob;
import pbsd.kde.plasma.runners.shell;

/// Burst 16 — Plasma runners.shell ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-workspace/runners/shell/shell.cpp
export namespace pbsd::kde::plasma::runners::shell::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct ShellRunnerStyle {
    const char* runner_id{shell::kRunnerId};
    const char* settings_svg{aero::kSettingsSvg};
    const char* shell_surface{kShellSurface};
};

[[nodiscard]] inline ShellRunnerStyle default_style() noexcept {
    return ShellRunnerStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::runners::shell::upstream_path();
}

} // namespace pbsd::kde::plasma::runners::shell::glue
