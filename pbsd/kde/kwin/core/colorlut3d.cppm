export module pbsd.kde.colorlut3d;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (colorlut3d.cpp).
/// Upstream: kde/kwin/src/core/colorlut3d.cpp
export namespace pbsd::kde::colorlut3d {

inline constexpr unsigned kLutSize = 33;
inline constexpr unsigned kChannelCount = 3;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/core/colorlut3d.cpp";
}

} // namespace pbsd::kde::colorlut3d
