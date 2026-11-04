export module pbsd.kde.plasma.systemlinks;

import pbsd.core;
import pbsd.kde.plasma.startmenu;

/// Wave 3 pass 6 — Start menu system link tiles (layout.json systemLinks).
/// Upstream: pbsd/theme/plasma/panel/layout.json
export namespace pbsd::kde::plasma::systemlinks {

    enum class Link : unsigned char {
        Documents, Pictures, ControlPanel, Music, Videos, Games, Computer, Network, Devices
    };
    inline constexpr unsigned kCount{9};
    inline constexpr const char kDocumentsSvg[] = "plasma/panel/documents.svg";
    inline constexpr const char kPicturesSvg[] = "plasma/panel/pictures.svg";
    inline constexpr const char kControlPanelSvg[] = "plasma/panel/control-panel.svg";
    inline constexpr const char kMusicSvg[] = "plasma/panel/music.svg";
    inline constexpr const char kVideosSvg[] = "plasma/panel/videos.svg";
    inline constexpr const char kGamesSvg[] = "plasma/panel/games.svg";
    inline constexpr const char kComputerSvg[] = "plasma/panel/computer.svg";
    inline constexpr const char kNetworkSvg[] = "plasma/panel/network.svg";
    inline constexpr const char kDevicesSvg[] = "plasma/panel/devices.svg";
    inline constexpr float kGlassOpacity{0.68f};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "pbsd/theme/plasma/panel/layout.json";
}

} // namespace pbsd::kde::plasma::systemlinks
