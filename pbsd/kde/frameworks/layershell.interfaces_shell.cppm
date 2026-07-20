export module pbsd.kde.layershell.interfaces_shell;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;

/// Wave 3 pass 4 — hand port constants (shell.cpp).
/// Upstream: kde/frameworks/layer-shell-qt/src/interfaces/shell.cpp
export namespace pbsd::kde::frameworks::layershell::interfaces_shell {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr int kVersion = 4;

[[nodiscard]] inline const char* shell_surface_name() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/interfaces/shell.cpp";
}

} // namespace pbsd::kde::frameworks::layershell::interfaces_shell
