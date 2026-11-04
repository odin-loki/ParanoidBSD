export module pbsd.kde.plasma.emojier;

import pbsd.core;

/// Wave 3 pass 4 — Emoji category ids.
/// Upstream: kde/plasma-desktop/emojier/emojicategory.cpp
export namespace pbsd::kde::plasma::emojier {

    inline constexpr const char kSmileys[] = "smileys";
    inline constexpr const char kPeople[] = "people";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/emojier/emojicategory.cpp";
}

} // namespace pbsd::kde::plasma::emojier
