export module pbsd.kde.kwin.scene.logging;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 4 — KWin logging category mirror.
/// Upstream: kde/kwin/src/scene/itemrenderer.cpp
export namespace pbsd::kde::kwin::scene::logging {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_SCENE",
    "kwin_scene",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/scene/itemrenderer.cpp",
};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/scene/itemrenderer.cpp";
}

} // namespace pbsd::kde::kwin::scene::logging
