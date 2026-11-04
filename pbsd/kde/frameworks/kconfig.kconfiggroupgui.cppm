export module pbsd.kde.frameworks.kconfig.kconfiggroupgui;

import pbsd.core;

/// Wave 3 — hand port constants (kconfiggroupgui.cpp).
/// Upstream: kde/frameworks/kconfig/src/gui/kconfiggroupgui.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfiggroupgui {

inline constexpr const char kGroupSeparator[] = "::";
inline constexpr unsigned kMaxGroupDepth = 16;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kconfiggroupgui.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfiggroupgui
