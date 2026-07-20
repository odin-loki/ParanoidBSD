export module pbsd.kde.kwin.lidswitchtracker;

import pbsd.core;

/// Wave 3 pass 4 — Lid switch state tracker constants.
/// Upstream: kde/kwin/src/lidswitchtracker.cpp
export namespace pbsd::kde::kwin::lidswitchtracker {

    inline constexpr const char kLidClosedKey[] = "lidClosed";
    inline constexpr const char kInhibitedKey[] = "inhibited";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/lidswitchtracker.cpp";
}

} // namespace pbsd::kde::kwin::lidswitchtracker
