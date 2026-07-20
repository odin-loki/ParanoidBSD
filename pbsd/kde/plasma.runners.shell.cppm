export module pbsd.kde.plasma.runners.shell;

import pbsd.core;

/// Burst 17 — Plasma shell runner constants.
/// Upstream: kde/plasma-workspace/runners/shell/shell.cpp
export namespace pbsd::kde::plasma::runners::shell {

inline constexpr const char kRunnerId[] = "shell";
inline constexpr const char kCommandPrefix[] = "!";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/runners/shell/shell.cpp";
}

} // namespace pbsd::kde::plasma::runners::shell
