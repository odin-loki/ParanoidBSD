export module pbsd.kde.plasma.dialogs;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 6 — Plasma dialog chrome constants (Aero glass dialogs).
/// Upstream: pbsd/theme/plasma/theme-colors.json
export namespace pbsd::kde::plasma::dialogs {

    enum class Kind : unsigned char {
        MessageBox, FileDialog, Progress, Wizard, Properties, About, InputBox, Conflict
    };
    inline constexpr const char kMessageBoxSvg[] = "plasma/dialogs/messagebox.svg";
    inline constexpr const char kFileDialogSvg[] = "plasma/dialogs/filedialog.svg";
    inline constexpr const char kProgressSvg[] = "plasma/dialogs/progress.svg";
    inline constexpr const char kWizardSvg[] = "plasma/dialogs/wizard.svg";
    inline constexpr const char kPropertiesSvg[] = "plasma/dialogs/properties.svg";
    inline constexpr const char kAboutSvg[] = "plasma/dialogs/about.svg";
    inline constexpr const char kInputBoxSvg[] = "plasma/dialogs/inputbox.svg";
    inline constexpr const char kConflictSvg[] = "plasma/dialogs/conflict.svg";
    inline constexpr float kDefaultOpacity{0.78f};
    inline constexpr int kDefaultCornerRadius{8};
    inline constexpr int kTitleBarHeight{36};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "pbsd/theme/plasma/theme-colors.json";
}

} // namespace pbsd::kde::plasma::dialogs
