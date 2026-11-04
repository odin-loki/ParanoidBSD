export module pbsd.kde.frameworks.kconfig.kauthorized;

import pbsd.core;

/// Wave 3 — KAuthorized action identifiers (hosted constants).
/// Upstream: kde/frameworks/kconfig/src/core/kauthorized.cpp
export namespace pbsd::kde::frameworks::kconfig::kauthorized {

inline constexpr const char kShellAccess[] = "shell_access";
inline constexpr const char kRunCommand[] = "run_command";
inline constexpr const char kRunCommandOnDesktop[] = "run_command_on_desktop";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/core/kauthorized.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kauthorized
