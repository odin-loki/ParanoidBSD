export module pbsd.kde.kpackage_effect;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (effect.cpp).
/// Upstream: kde/kwin/src/plugins/kpackage/effect/effect.cpp
export namespace pbsd::kde::kpackage_effect {

inline constexpr const char kEffectsDir[] = "kwin/effects";
inline constexpr const char kMetadataFile[] = "metadata.json";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/kpackage/effect/effect.cpp";
}

} // namespace pbsd::kde::kpackage_effect
