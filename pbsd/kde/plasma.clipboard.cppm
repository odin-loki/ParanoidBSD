export module pbsd.kde.plasma.clipboard;

import pbsd.core;
import pbsd.kde.plasma.aero;

/// Burst 16 — Plasma clipboard constants.
/// Upstream: kde/plasma-workspace/applets/clipboard/clipboard.cpp
export namespace pbsd::kde::plasma::clipboard {

inline constexpr const char kAppletId[] = "org.kde.plasma.clipboard";
inline constexpr unsigned kMaxHistory{20};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-workspace/applets/clipboard/clipboard.cpp";
}

} // namespace pbsd::kde::plasma::clipboard
