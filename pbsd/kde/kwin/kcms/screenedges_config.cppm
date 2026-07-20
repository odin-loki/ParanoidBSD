export module pbsd.kde.screenedges_config;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (kwintouchscreenedgeconfigform.cpp).
/// Upstream: kde/kwin/src/kcms/screenedges/kwintouchscreenedgeconfigform.cpp
export namespace pbsd::kde::screenedges_config {

inline constexpr const char kScreenEdgesGroup[] = "ScreenEdges";
inline constexpr unsigned kEdgeCount = 4;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/kcms/screenedges/kwintouchscreenedgeconfigform.cpp";
}

} // namespace pbsd::kde::screenedges_config
