export module pbsd.kde.plasma.shell;

import pbsd.core;

/// Wave 3 pass 5 — Plasma workspace shell service ids.
/// Upstream: kde/plasma-workspace/shell/shell.cpp
export namespace pbsd::kde::plasma::shell {

    inline constexpr const char kShellDBusService[] = "org.kde.plasmashell";
    inline constexpr const char kShellObjectPath[] = "/PlasmaShell";
    inline constexpr const char kDesktopContainment[] = "Desktop";
    inline constexpr const char kPanelContainment[] = "Panel";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/shell/shell.cpp";
}

} // namespace pbsd::kde::plasma::shell
