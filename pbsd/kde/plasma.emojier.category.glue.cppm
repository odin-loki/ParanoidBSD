export module pbsd.kde.plasma.emojier.category.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.emojier.category;

/// Burst 16 wave 2 — Plasma emojier.category ↔ Aero glue.
/// Upstream: kde/plasma-desktop/applets/emojier/category.cpp
/// layershell: string constants only (no layershellintegration import — CMake cycle).
export namespace pbsd::kde::plasma::emojier::category::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct EmojiCategoryStyle {
    unsigned max_recent{32};
    float popup_opacity{0.88f};
};

[[nodiscard]] inline EmojiCategoryStyle default_style() noexcept { return EmojiCategoryStyle{}; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::emojier::category::upstream_path();
}

} // namespace pbsd::kde::plasma::emojier::category::glue
