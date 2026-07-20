export module pbsd.kde.frameworks.layershell.shell;

import pbsd.core;
import pbsd.kde.layershell.layershellintegration;

/// Wave 3 pass 3 — Layer-shell Qt interface stub.
/// Upstream: kde/frameworks/layer-shell-qt/src/interfaces/shell.cpp
export namespace pbsd::kde::frameworks::layershell::shell {

    inline constexpr const char kInterfaceId[] = "org.kde.layershell";

[[nodiscard]] inline const char* integration_plugin() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kIntegrationPlugin;
}

[[nodiscard]] inline const char* shell_surface() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::kShellSurface;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::frameworks::layershell::layershellintegration::upstream_path();
}

} // namespace pbsd::kde::frameworks::layershell::shell
