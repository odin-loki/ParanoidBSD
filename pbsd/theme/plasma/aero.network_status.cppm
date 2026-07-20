export module pbsd.theme.plasma.aero.network_status;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 15 — Aero network status visual tokens.
export namespace pbsd::theme::plasma::aero::network_status {

struct NetworkTokens {
    unsigned icon_size_px{16};
    float offline_opacity{0.55f};
    const char* network_svg{"plasma/panel/network.svg"};
};

[[nodiscard]] inline NetworkTokens default_tokens() noexcept {
    return NetworkTokens{};
}

} // namespace pbsd::theme::plasma::aero::network_status
