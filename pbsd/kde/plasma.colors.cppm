export module pbsd.kde.plasma.colors;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Wave 3 pass 6 — PBSDAero accent palette (brand blue, no purple slop).
/// Upstream: pbsd/theme/plasma/color-schemes/PBSDAero.colors
export namespace pbsd::kde::plasma::colors {

    enum class AccentRole : unsigned char {
        Primary, PrimaryLight, PrimaryDark, Highlight, Hover, FocusRing, ShutdownHover
    };
    struct Rgb { unsigned char r; unsigned char g; unsigned char b; };
    inline constexpr Rgb kPrimary{0x50, 0x82, 0xc8};
    inline constexpr Rgb kPrimaryLight{0x78, 0xa8, 0xe8};
    inline constexpr Rgb kPrimaryDark{0x2a, 0x50, 0x88};
    inline constexpr Rgb kHighlight{0xb8, 0xd8, 0xff};
    inline constexpr Rgb kHover{0x78, 0xa8, 0xe8};
    inline constexpr Rgb kFocusRing{0x50, 0x82, 0xc8};
    inline constexpr Rgb kShutdownHover{0xe8, 0x78, 0x30};
    inline constexpr const char kColorSchemeName[] = "PBSDAero";
    inline constexpr const char kColorSchemePath[] = "plasma/color-schemes/PBSDAero.colors";
    [[nodiscard]] inline Rgb accent(AccentRole role) noexcept {
        switch (role) {
        case AccentRole::Primary: return kPrimary;
        case AccentRole::PrimaryLight: return kPrimaryLight;
        case AccentRole::PrimaryDark: return kPrimaryDark;
        case AccentRole::Highlight: return kHighlight;
        case AccentRole::Hover: return kHover;
        case AccentRole::FocusRing: return kFocusRing;
        case AccentRole::ShutdownHover: return kShutdownHover;
        }
        return kPrimary;
    }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "pbsd/theme/plasma/color-schemes/PBSDAero.colors";
}

} // namespace pbsd::kde::plasma::colors
