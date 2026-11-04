export module pbsd.kde.kwin.wayland.xdg_shell;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 7 — xdg-shell stable protocol glue (KWin Wayland).
/// Upstream: kde/kwin/src/wayland/xdgshell.cpp
export namespace pbsd::kde::kwin::wayland::xdg_shell {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_XDG_SHELL",
    "kwin_xdg_shell",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/xdgshell.cpp",
};

inline constexpr const char kInterface[] = "xdg_wm_base";
inline constexpr int kVersion = 6;

enum class Role : unsigned char {
    Toplevel,
    Popup,
    Positioner,
};

enum class DecorationMode : unsigned char {
    ClientSide = 1,
    ServerSide = 2,
};

struct ToplevelState {
    int width{0};
    int height{0};
    bool maximized{false};
    bool fullscreen{false};
    DecorationMode decoration{DecorationMode::ServerSide};
};

[[nodiscard]] inline bool is_valid_role(Role role) noexcept {
    return role <= Role::Positioner;
}

[[nodiscard]] inline bool prefers_server_decoration(DecorationMode mode) noexcept {
    return mode == DecorationMode::ServerSide;
}

[[nodiscard]] inline Status validate_toplevel_size(int w, int h) noexcept {
    if (w < 0 || h < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/xdgshell.cpp";
}

} // namespace pbsd::kde::kwin::wayland::xdg_shell
