export module pbsd.kde.frameworks.kconfig.kwindowstatesaverquick;

import pbsd.core;
import pbsd.kde.frameworks.kconfig.kwindowstatesaver;

/// Wave 3 — QML window state saver bridge constants.
/// Upstream: kde/frameworks/kconfig/src/qml/kwindowstatesaverquick.cpp
export namespace pbsd::kde::frameworks::kconfig::kwindowstatesaverquick {

inline constexpr const char kQmlModule[] = "org.kde.kconfig";
inline constexpr const char kTypeName[] = "KWindowStateSaver";

[[nodiscard]] inline const char* width_key() noexcept { return kwindowstatesaver::kWidthKey; }

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kconfig/src/qml/kwindowstatesaverquick.cpp";
}

} // namespace pbsd::kde::frameworks::kconfig::kwindowstatesaverquick
