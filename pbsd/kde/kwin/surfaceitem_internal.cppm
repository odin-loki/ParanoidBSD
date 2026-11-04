export module pbsd.kde.kwin.surfaceitem_internal;

import pbsd.core;

/// Wave 3 pass 4 — Internal surface item flags.
/// Upstream: kde/kwin/src/scene/surfaceitem_internal.cpp
export namespace pbsd::kde::kwin::surfaceitem_internal {

    inline constexpr unsigned kInternalFlag = 0x01;
    inline constexpr unsigned kScanoutCandidate = 0x02;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/scene/surfaceitem_internal.cpp";
}

} // namespace pbsd::kde::kwin::surfaceitem_internal
