export module pbsd.kde.plasma.aero;

import pbsd.core;

/// Wave 3 — Aero theme constants shared by Plasma shell and KWin stubs.
export namespace pbsd::kde::plasma::aero {

inline constexpr const char kThemeId[] = "pbsd-aero";
inline constexpr const char kColorScheme[] = "PBSDAero";
inline constexpr const char kMetadataPath[] = "plasma/metadata.json";
inline constexpr const char kColorsPath[] = "plasma/colors";
inline constexpr const char kColorSchemePath[] = "plasma/color-schemes/PBSDAero.colors";
inline constexpr const char kPlasmarcPath[] = "plasma/plasmarc";
inline constexpr const char kBlurEffectPath[] = "plasma/blur-effect.json";
inline constexpr const char kDecorationId[] = "pbsd_aero";
inline constexpr const char kWidgetsDir[] = "plasma/widgets";
inline constexpr const char kDialogsDir[] = "plasma/dialogs";
inline constexpr const char kDecorationDir[] = "plasma/decoration";
inline constexpr const char kPanelDir[] = "plasma/panel";
inline constexpr const char kPanelLayoutPath[] = "plasma/panel/layout.json";
inline constexpr const char kTaskbarSvg[] = "plasma/panel/taskbar.svg";
inline constexpr const char kStartButtonSvg[] = "plasma/panel/start-button.svg";
inline constexpr const char kThemeColorsPath[] = "plasma/theme-colors.json";
inline constexpr const char kDecorationConfigPath[] = "plasma/decoration/config.json";
inline constexpr const char kClockSvg[] = "plasma/panel/clock.svg";
inline constexpr const char kShowDesktopSvg[] = "plasma/panel/show-desktop.svg";
inline constexpr const char kNotificationSvg[] = "plasma/widgets/notification.svg";
inline constexpr const char kMessageBoxSvg[] = "plasma/dialogs/messagebox.svg";
inline constexpr const char kStartMenuNotesPath[] = "plasma/panel/start-menu.md";
inline constexpr const char kStartMenuSvg[] = "plasma/panel/start-menu.svg";
inline constexpr const char kApplicationsSvg[] = "plasma/panel/applications.svg";
inline constexpr const char kMenuItemSvg[] = "plasma/widgets/menuitem.svg";
inline constexpr const char kProgressSvg[] = "plasma/widgets/progress.svg";
inline constexpr const char kSpinnerSvg[] = "plasma/widgets/spinner.svg";
inline constexpr const char kHoverHighlightSvg[] = "plasma/widgets/hover-highlight.svg";
inline constexpr const char kSearchBoxSvg[] = "plasma/panel/search-box.svg";
inline constexpr const char kShutdownSvg[] = "plasma/panel/shutdown.svg";
inline constexpr const char kUserTileSvg[] = "plasma/panel/user-tile.svg";
inline constexpr const char kSidebarSvg[] = "plasma/panel/sidebar.svg";
inline constexpr const char kStartMenuFrameSvg[] = "plasma/panel/start-menu-frame.svg";
inline constexpr const char kPanelGlassSvg[] = "plasma/panel/panel-glass.svg";
inline constexpr const char kDocumentsSvg[] = "plasma/panel/documents.svg";
inline constexpr const char kPicturesSvg[] = "plasma/panel/pictures.svg";
inline constexpr const char kControlPanelSvg[] = "plasma/panel/control-panel.svg";
inline constexpr const char kTabSvg[] = "plasma/widgets/tab.svg";
inline constexpr const char kCheckboxSvg[] = "plasma/widgets/checkbox.svg";
inline constexpr const char kRadiobuttonSvg[] = "plasma/widgets/radiobutton.svg";
inline constexpr const char kQuickLaunchSvg[] = "plasma/panel/quick-launch.svg";
inline constexpr const char kFileDialogSvg[] = "plasma/dialogs/filedialog.svg";
inline constexpr const char kProgressDialogSvg[] = "plasma/dialogs/progress.svg";
inline constexpr const char kWizardDialogSvg[] = "plasma/dialogs/wizard.svg";
inline constexpr const char kPropertiesDialogSvg[] = "plasma/dialogs/properties.svg";
inline constexpr const char kAboutDialogSvg[] = "plasma/dialogs/about.svg";
inline constexpr const char kComboboxSvg[] = "plasma/widgets/combobox.svg";
inline constexpr const char kListboxSvg[] = "plasma/widgets/listbox.svg";
inline constexpr const char kTreeviewSvg[] = "plasma/widgets/treeview.svg";
inline constexpr const char kVolumeSvg[] = "plasma/widgets/volume.svg";
inline constexpr const char kBatterySvg[] = "plasma/widgets/battery.svg";
inline constexpr const char kWifiSvg[] = "plasma/widgets/wifi.svg";
inline constexpr const char kBluetoothSvg[] = "plasma/widgets/bluetooth.svg";
inline constexpr const char kMusicSvg[] = "plasma/panel/music.svg";
inline constexpr const char kVideosSvg[] = "plasma/panel/videos.svg";
inline constexpr const char kGamesSvg[] = "plasma/panel/games.svg";
inline constexpr const char kComputerSvg[] = "plasma/panel/computer.svg";
inline constexpr const char kNetworkSvg[] = "plasma/panel/network.svg";
inline constexpr const char kDevicesSvg[] = "plasma/panel/devices.svg";
inline constexpr const char kFavoritesSvg[] = "plasma/panel/favorites.svg";
inline constexpr const char kRecentSvg[] = "plasma/panel/recent.svg";
inline constexpr const char kAllProgramsSvg[] = "plasma/panel/all-programs.svg";
inline constexpr const char kPowerButtonSvg[] = "plasma/panel/power-button.svg";
inline constexpr const char kLockSvg[] = "plasma/panel/lock.svg";
inline constexpr const char kSleepSvg[] = "plasma/panel/sleep.svg";
inline constexpr const char kRestartSvg[] = "plasma/panel/restart.svg";
inline constexpr const char kLogoffSvg[] = "plasma/panel/logoff.svg";
inline constexpr const char kHelpSvg[] = "plasma/panel/help.svg";
inline constexpr const char kSettingsSvg[] = "plasma/panel/settings.svg";
inline constexpr const char kRunSvg[] = "plasma/panel/run.svg";
inline constexpr const char kSearchSvg[] = "plasma/panel/search.svg";
inline constexpr const char kBorderTopSvg[] = "plasma/decoration/border-top.svg";
inline constexpr const char kBorderRightSvg[] = "plasma/decoration/border-right.svg";
inline constexpr const char kBorderBottomSvg[] = "plasma/decoration/border-bottom.svg";
inline constexpr const char kGlowActiveSvg[] = "plasma/decoration/glow-active.svg";
inline constexpr const char kGlowInactiveSvg[] = "plasma/decoration/glow-inactive.svg";
inline constexpr const char kResizeGripSvg[] = "plasma/decoration/resize-grip.svg";
inline constexpr const char kThemeVersion[] = "0.8.0";
inline constexpr const char kAccentPrimaryHex[] = "#5082c8";
inline constexpr const char kAccentHighlightHex[] = "#b8d8ff";
inline constexpr int kGlowBlurRadius{20};
inline constexpr float kGlowSaturation{1.20f};
inline constexpr float kSystemLinkOpacity{0.68f};

enum class CompositingSurface : unsigned char {
    Panel, Dialog, StartMenu, Decoration, Glow, SystemLink
};

struct GlassTint {
    float r{0.12f};
    float g{0.22f};
    float b{0.40f};
    float a{0.65f};
};

struct BlurParams {
    GlassTint tint{};
    int blur_radius{24};
    float noise{0.04f};
    float saturation{1.15f};
    float contrast{1.05f};
    float panel_opacity{0.72f};
    float dialog_opacity{0.78f};
    int title_bar_height{36};
    int corner_radius{8};
};

[[nodiscard]] inline BlurParams default_blur() noexcept { return BlurParams{}; }

[[nodiscard]] inline Status validate_theme_id(const char* id) noexcept {
    if (id == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline bool theme_id_matches(const char* id) noexcept {
    if (id == nullptr) {
        return false;
    }
    return id[0] == 'p' && id[1] == 'b' && id[2] == 's' && id[3] == 'd'
        && id[4] == '-' && id[5] == 'a' && id[6] == 'e' && id[7] == 'r'
        && id[8] == 'o' && id[9] == '\0';
}

} // namespace pbsd::kde::plasma::aero
