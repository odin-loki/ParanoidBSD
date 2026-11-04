export module pbsd.kde.layershell.interfaces_window;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;

/// Burst 15 — layer-shell-qt window interface stub.
/// Upstream: kde/frameworks/layer-shell-qt/src/interfaces/window.cpp
export namespace pbsd::kde::frameworks::layershell::interfaces_window {

inline constexpr const char kInterfaceId[] = "org.kde.layershell.window";

[[nodiscard]] inline const char* shell_surface() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface;
}

[[nodiscard]] inline const char* integration_plugin() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/interfaces/window.cpp";
}

} // namespace pbsd::kde::frameworks::layershell::interfaces_window
