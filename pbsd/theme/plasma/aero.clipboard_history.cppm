export module pbsd.theme.plasma.aero.clipboard_history;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Aero clipboard history visual tokens.
export namespace pbsd::theme::plasma::aero::clipboard_history {

struct ClipboardTokens {
    unsigned max_rows{8};
    unsigned row_height_px{28};
    float popup_opacity{0.82f};
};

[[nodiscard]] inline ClipboardTokens default_tokens() noexcept {
    return ClipboardTokens{};
}

} // namespace pbsd::theme::plasma::aero::clipboard_history
