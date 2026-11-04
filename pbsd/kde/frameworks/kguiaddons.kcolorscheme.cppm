export module pbsd.kde.frameworks.kguiaddons.kcolorscheme;

import pbsd.core;

/// Wave 9 burst — KColorScheme semantic roles.
/// Upstream: kde/frameworks/kguiaddons/src/colorscheme/kcolorscheme.cpp
export namespace pbsd::kde::frameworks::kguiaddons::kcolorscheme {

enum class ColorSet : unsigned char {
    Window,
    Button,
    View,
    Selection,
    Tooltip,
    Complementary,
};

enum class ShadeRole : unsigned char {
    Background,
    Foreground,
    Decoration,
};

inline constexpr int kMaxShades = 7;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kguiaddons/src/colorscheme/kcolorscheme.cpp";
}

} // namespace pbsd::kde::frameworks::kguiaddons::kcolorscheme
