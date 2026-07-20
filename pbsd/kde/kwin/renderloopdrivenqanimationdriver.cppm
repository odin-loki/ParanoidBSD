export module pbsd.kde.renderloopdrivenqanimationdriver;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (renderloopdrivenqanimationdriver.cpp).
/// Upstream: kde/kwin/src/renderloopdrivenqanimationdriver.cpp
export namespace pbsd::kde::renderloopdrivenqanimationdriver {

inline constexpr unsigned kDefaultFps = 60;
inline constexpr unsigned kMinFrameMs = 16;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/renderloopdrivenqanimationdriver.cpp";
}

} // namespace pbsd::kde::renderloopdrivenqanimationdriver
