export module pbsd.theme.plasma.aero.jump_list;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 15 — Aero jump list visual tokens.
export namespace pbsd::theme::plasma::aero::jump_list {

struct JumpListTokens {
    unsigned max_items{10};
    unsigned row_height_px{28};
    float popup_opacity{0.78f};
    const char* jumplist_svg{"plasma/panel/jump-list.svg"};
};

[[nodiscard]] inline JumpListTokens default_tokens() noexcept {
    return JumpListTokens{};
}

} // namespace pbsd::theme::plasma::aero::jump_list
