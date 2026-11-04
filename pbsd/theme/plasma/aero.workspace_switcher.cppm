export module pbsd.theme.plasma.aero.workspace_switcher;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Aero workspace switcher visual tokens.
export namespace pbsd::theme::plasma::aero::workspace_switcher {

struct WorkspaceTokens {
    unsigned thumbnail_width_px{240};
    unsigned animation_ms{180};
    float dim_opacity{0.30f};
};

[[nodiscard]] inline WorkspaceTokens default_tokens() noexcept {
    return WorkspaceTokens{};
}

} // namespace pbsd::theme::plasma::aero::workspace_switcher
