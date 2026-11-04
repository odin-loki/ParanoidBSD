export module pbsd.kde.plasma.applet_packagestructure;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (plasma_applet_packagestructure.cpp).
/// Upstream: kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_applet_packagestructure.cpp
export namespace pbsd::kde::frameworks::plasma::applet_packagestructure {

inline constexpr const char kPluginId[] = "Plasma/Applet";
inline constexpr const char kConfigModel[] = "config/config.qml";
inline constexpr const char kMainConfigXml[] = "config/main.xml";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasma/packagestructure/plasma_applet_packagestructure.cpp";
}

} // namespace pbsd::kde::frameworks::plasma::applet_packagestructure
