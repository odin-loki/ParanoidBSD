export module pbsd.kde.x11eventfilter;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (x11eventfilter.cpp).
/// Upstream: kde/kwin/src/x11eventfilter.cpp
export namespace pbsd::kde::x11eventfilter {

inline constexpr unsigned kClientMessage = 33;
inline constexpr unsigned kPropertyNotify = 28;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/x11eventfilter.cpp";
}

} // namespace pbsd::kde::x11eventfilter
