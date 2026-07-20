export module pbsd.kde.frameworks.kconfig.kconfiggui;

import pbsd.core;

/// Wave 3 — KConfigGui module identity (from kconfiggui.cpp).
/// Upstream: kde/frameworks/kconfig/src/gui/kconfiggui.cpp
export namespace pbsd::kde::frameworks::kconfig::kconfiggui {

inline constexpr const char kModuleName[] = "KConfigGui";
inline constexpr const char kVersion[] = "6.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/gui/kconfiggui.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kconfiggui
