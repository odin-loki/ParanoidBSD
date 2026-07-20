export module pbsd.kde.plasma.wheelinterceptor;

import pbsd.core;

/// Wave 3 pass 4 — Wheel scroll delta cap.
/// Upstream: kde/plasma-desktop/containments/desktop/plugins/folder/wheelinterceptor.cpp
export namespace pbsd::kde::plasma::wheelinterceptor {

    inline constexpr int kMaxDelta = 120;
    inline constexpr int kMinDelta = -120;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/containments/desktop/plugins/folder/wheelinterceptor.cpp";
}

} // namespace pbsd::kde::plasma::wheelinterceptor
