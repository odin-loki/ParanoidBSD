export module pbsd.kde.plasma.emojier.category;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (emojicategory.cpp).
/// Upstream: kde/plasma-desktop/emojier/emojicategory.cpp
export namespace pbsd::kde::plasma::emojier::category {

inline constexpr const char kRecentCategory[] = "recent";
inline constexpr const char kSmileysCategory[] = "smileys";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/emojier/emojicategory.cpp";
}

} // namespace pbsd::kde::plasma::emojier::category
