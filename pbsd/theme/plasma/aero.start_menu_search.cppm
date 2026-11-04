export module pbsd.theme.plasma.aero.start_menu_search;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 14 — Aero start menu search visual tokens.
export namespace pbsd::theme::plasma::aero::start_menu_search {

struct SearchTokens {
    const char* placeholder{"Search apps, files, settings"};
    unsigned result_limit{24};
    const char* search_box_svg{"plasma/panel/search-box.svg"};
};

[[nodiscard]] inline SearchTokens default_tokens() noexcept {
    return SearchTokens{};
}

} // namespace pbsd::theme::plasma::aero::start_menu_search
