export module pbsd.kde.plasma.folder.applauncher;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (applauncher.cpp).
/// Upstream: kde/plasma-desktop/containments/desktop/plugins/folder/applauncher.cpp
export namespace pbsd::kde::plasma::folder::applauncher {

inline constexpr const char kDesktopEntrySuffix[] = ".desktop";
inline constexpr unsigned kMaxApps = 256;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/containments/desktop/plugins/folder/applauncher.cpp";
}

} // namespace pbsd::kde::plasma::folder::applauncher
