export module pbsd.kde.plasma.decoration.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 11 — Plasma shell ↔ KDecoration3 pbsd_aero asset paths.
/// Upstream: kde/plasma-desktop/shells/plasma5/shell.cpp
export namespace pbsd::kde::plasma::decoration::glue {

struct DecorationBinding {
    const char* plugin_id{aero::kDecorationId};
    const char* metadata_path{aero::kMetadataPath};
    const char* config_path{aero::kDecorationConfigPath};
    const char* decoration_dir{aero::kDecorationDir};
    int title_bar_height{aero::default_blur().title_bar_height};
    int corner_radius{aero::default_blur().corner_radius};
};

[[nodiscard]] inline DecorationBinding aero_binding() noexcept {
    return DecorationBinding{};
}

[[nodiscard]] inline bool plugin_matches(const char* id) noexcept {
    return aero::theme_id_matches(id) || (id != nullptr && id[0] == 'p');
}

[[nodiscard]] inline Status validate_title_bar_height(int h) noexcept {
    if (h < 24 || h > 64) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/shells/plasma5/shell.cpp";
}

} // namespace pbsd::kde::plasma::decoration::glue
