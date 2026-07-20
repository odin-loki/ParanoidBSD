export module pbsd.kde.frameworks.kconfig.kwindowstatesaver;

import pbsd.core;

/// Wave 3 — window geometry/state persistence keys.
/// Upstream: kde/frameworks/kconfig/src/gui/kwindowstatesaver.cpp
export namespace pbsd::kde::frameworks::kconfig::kwindowstatesaver {

inline constexpr const char kWidthKey[] = "Width";
inline constexpr const char kHeightKey[] = "Height";
inline constexpr const char kXKey[] = "X";
inline constexpr const char kYKey[] = "Y";
inline constexpr const char kMaximizedKey[] = "Maximized";
inline constexpr const char kFullscreenKey[] = "Fullscreen";

struct Geometry {
    int x{0};
    int y{0};
    int width{0};
    int height{0};
    bool maximized{false};
    bool fullscreen{false};
};

[[nodiscard]] inline bool geometry_valid(const Geometry& g) noexcept {
    return g.width > 0 && g.height > 0;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kwindowstatesaver.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kwindowstatesaver
