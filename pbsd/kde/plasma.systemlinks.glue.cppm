export module pbsd.kde.plasma.systemlinks.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.systemlinks;

/// Burst 13 — Start menu system link tiles ↔ Aero asset glue.
/// Upstream: pbsd/theme/plasma/panel/layout.json
export namespace pbsd::kde::plasma::systemlinks::glue {

struct LinkTile {
    systemlinks::Link link{systemlinks::Link::Documents};
    const char* svg{systemlinks::kDocumentsSvg};
    float glass_opacity{systemlinks::kGlassOpacity};
};

[[nodiscard]] inline LinkTile tile_for(systemlinks::Link link) noexcept {
    LinkTile t{};
    t.link = link;
    switch (link) {
    case systemlinks::Link::Pictures:
        t.svg = systemlinks::kPicturesSvg;
        break;
    case systemlinks::Link::ControlPanel:
        t.svg = systemlinks::kControlPanelSvg;
        break;
    case systemlinks::Link::Music:
        t.svg = systemlinks::kMusicSvg;
        break;
    case systemlinks::Link::Videos:
        t.svg = systemlinks::kVideosSvg;
        break;
    case systemlinks::Link::Games:
        t.svg = systemlinks::kGamesSvg;
        break;
    case systemlinks::Link::Computer:
        t.svg = systemlinks::kComputerSvg;
        break;
    case systemlinks::Link::Network:
        t.svg = systemlinks::kNetworkSvg;
        break;
    case systemlinks::Link::Devices:
        t.svg = systemlinks::kDevicesSvg;
        break;
    default:
        t.svg = systemlinks::kDocumentsSvg;
        break;
    }
    return t;
}

[[nodiscard]] inline Status validate_link_index(unsigned idx) noexcept {
    return idx < systemlinks::kCount ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* sidebar_svg() noexcept {
    return aero::kSidebarSvg;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return systemlinks::upstream_path();
}

} // namespace pbsd::kde::plasma::systemlinks::glue
