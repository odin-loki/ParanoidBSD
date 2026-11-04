export module pbsd.kde.kwin.backgroundeffectitem;

import pbsd.core;

/// Wave 3 pass 4 — Background effect item layer.
/// Upstream: kde/kwin/src/scene/backgroundeffectitem.cpp
export namespace pbsd::kde::kwin::backgroundeffectitem {

    inline constexpr int kBackgroundLayer = -1;
    inline constexpr float kDefaultOpacity = 1.0f;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/scene/backgroundeffectitem.cpp";
}

} // namespace pbsd::kde::kwin::backgroundeffectitem
