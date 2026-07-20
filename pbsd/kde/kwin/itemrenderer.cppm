export module pbsd.kde.kwin.itemrenderer;

import pbsd.core;

/// Wave 3 pass 4 — Scene item renderer backend ids.
/// Upstream: kde/kwin/src/scene/itemrenderer.cpp
export namespace pbsd::kde::kwin::itemrenderer {

    inline constexpr const char kOpenGlRenderer[] = "opengl";
    inline constexpr const char kQPainterRenderer[] = "qpainter";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/scene/itemrenderer.cpp";
}

} // namespace pbsd::kde::kwin::itemrenderer
