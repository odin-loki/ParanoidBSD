export module pbsd.kde.plasma.runners.bookmarks;

import pbsd.core;

/// Burst 18 — Plasma bookmarks runner constants.
/// Upstream: kde/plasma-workspace/runners/bookmarks/bookmarks.cpp
export namespace pbsd::kde::plasma::runners::bookmarks {

inline constexpr const char kRunnerId[] = "bookmarks";
inline constexpr const char kTrigger[] = "bm:";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/runners/bookmarks/bookmarks.cpp";
}

} // namespace pbsd::kde::plasma::runners::bookmarks
