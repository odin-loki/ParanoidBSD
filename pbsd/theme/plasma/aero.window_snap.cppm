export module pbsd.theme.plasma.aero.window_snap;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 14 — Aero window snap visual tokens.
export namespace pbsd::theme::plasma::aero::window_snap {

struct SnapTokens {
    unsigned snap_margin_px{8};
    unsigned preview_border_px{2};
    float preview_opacity{0.45f};
};

[[nodiscard]] inline SnapTokens default_tokens() noexcept {
    return SnapTokens{};
}

} // namespace pbsd::theme::plasma::aero::window_snap
