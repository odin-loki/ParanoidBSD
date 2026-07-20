export module pbsd.kde.plasma.knetattach.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.knetattach;

/// Burst 15 — Plasma knetattach ↔ Aero glue.
/// Upstream: kde/plasma-desktop/knetattach/main.cpp
export namespace pbsd::kde::plasma::knetattach::glue {

struct NetworkAttachStyle {
    const char* app_id{knetattach::kAppId};
    const char* protocol_key{knetattach::kProtocolKey};
    const char* settings_svg{aero::kSettingsSvg};
};

[[nodiscard]] inline NetworkAttachStyle default_style() noexcept {
    return NetworkAttachStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::knetattach::upstream_path();
}

} // namespace pbsd::kde::plasma::knetattach::glue
