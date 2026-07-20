export module pbsd.kde.kwin.rootitem;

import pbsd.core;

/// Wave 3 pass 3 — Scene root item layer id.
/// Upstream: kde/kwin/src/scene/rootitem.cpp
export namespace pbsd::kde::kwin::rootitem {

    inline constexpr int kRootLayer{0};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/scene/rootitem.cpp";
}

} // namespace pbsd::kde::kwin::rootitem
