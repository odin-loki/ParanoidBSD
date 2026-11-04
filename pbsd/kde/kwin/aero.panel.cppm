export module pbsd.kde.kwin.aero.panel;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.kwin.compositing;

/// Wave 3 pass 6 — KWin panel glass compositing bridge (panel opacity + blur).
/// Upstream: pbsd/theme/plasma/panel/layout.json
export namespace pbsd::kde::kwin::aero::panel {

    inline constexpr int kPanelHeight{40};
    inline constexpr float kPanelOpacity{0.72f};
    inline constexpr int kBlurRadius{24};
    inline constexpr const char kPanelGlassSvg[] = "plasma/panel/panel-glass.svg";
    enum class Edge : unsigned char { Top, Bottom, Left, Right };
    inline constexpr Edge kDefaultEdge{Edge::Bottom};
    [[nodiscard]] inline ::pbsd::kde::plasma::aero::BlurParams blur() noexcept {
        return ::pbsd::kde::kwin::compositing::panel_blur();
    }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "pbsd/theme/plasma/panel/layout.json";
}

} // namespace pbsd::kde::kwin::aero::panel
