export module pbsd.kde.layershell.qwaylandlayershellintegrationplugin;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;

/// Burst 15 — layer-shell-qt Wayland shell integration plugin stub.
/// Upstream: kde/frameworks/layer-shell-qt/src/qwaylandlayershellintegrationplugin.cpp
export namespace pbsd::kde::layershell::qwaylandlayershellintegrationplugin {

inline constexpr const char kPluginIid[] =
    "org.qt-project.Qt.WaylandClient.QWaylandShellIntegrationFactoryInterface.5.4";
inline constexpr const char kMetadataFile[] = "layer-shell.json";

[[nodiscard]] inline const char* integration_plugin() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin;
}

[[nodiscard]] inline const char* shell_surface() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/layer-shell-qt/src/qwaylandlayershellintegrationplugin.cpp";
}

} // namespace pbsd::kde::layershell::qwaylandlayershellintegrationplugin
