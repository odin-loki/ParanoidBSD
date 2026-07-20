export module pbsd.kde.dndiconitem;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (dndiconitem.cpp).
/// Upstream: kde/kwin/src/scene/dndiconitem.cpp
export namespace pbsd::kde::dndiconitem {

inline constexpr int kDefaultIconSize = 48;
inline constexpr float kDefaultOpacity = 0.85f;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/scene/dndiconitem.cpp";
}

} // namespace pbsd::kde::dndiconitem
