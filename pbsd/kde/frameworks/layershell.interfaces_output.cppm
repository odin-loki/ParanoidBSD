export module pbsd.kde.layershell.interfaces_output;

import pbsd.core;

/// Burst 16 wave 2 — layer-shell output interface stub (string constants).
export namespace pbsd::kde::frameworks::layershell::interfaces_output {

inline constexpr const char kInterfaceId[] = "org.kde.layershell.output";
inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/interfaces/output.cpp";
}

} // namespace pbsd::kde::frameworks::layershell::interfaces_output
