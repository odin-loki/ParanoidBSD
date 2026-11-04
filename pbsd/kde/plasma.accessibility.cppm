export module pbsd.kde.plasma.accessibility;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Plasma accessibility constants.
/// Upstream: kde/plasma-desktop/kcms/accessibility/accessibility.cpp
export namespace pbsd::kde::plasma::accessibility {

inline constexpr const char kKcmId[] = "kcm_accessibility";
inline constexpr float kMinContrast{1.0f};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/accessibility/accessibility.cpp";
}

} // namespace pbsd::kde::plasma::accessibility
