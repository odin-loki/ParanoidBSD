export module pbsd.kde.plasma.clipboard.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.clipboard;

/// Burst 16 — Plasma clipboard ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-workspace/applets/clipboard/clipboard.cpp
export namespace pbsd::kde::plasma::clipboard::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct ClipboardStyle {
    const char* applet_id{clipboard::kAppletId};
    unsigned max_history{clipboard::kMaxHistory};
    const char* shell_surface{kShellSurface};
    float popup_opacity{0.82f};
};

[[nodiscard]] inline ClipboardStyle default_style() noexcept {
    return ClipboardStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::clipboard::upstream_path();
}

} // namespace pbsd::kde::plasma::clipboard::glue
