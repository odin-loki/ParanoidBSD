export module pbsd.kde.plasma.menuhelper;

import pbsd.core;

/// Wave 3 pass 4 — Folder menu helper keys.
/// Upstream: kde/plasma-desktop/containments/desktop/plugins/folder/menuhelper.cpp
export namespace pbsd::kde::plasma::menuhelper {

    inline constexpr const char kNewFolderAction[] = "newFolder";
    inline constexpr const char kPasteAction[] = "paste";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/containments/desktop/plugins/folder/menuhelper.cpp";
}

} // namespace pbsd::kde::plasma::menuhelper
