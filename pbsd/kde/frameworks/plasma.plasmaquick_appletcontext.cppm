export module pbsd.kde.plasma.plasmaquick_appletcontext;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (appletcontext.cpp).
/// Upstream: kde/frameworks/plasma-framework/src/plasmaquick/appletcontext.cpp
export namespace pbsd::kde::frameworks::plasma::plasmaquick_appletcontext {

inline constexpr const char kContextProperty[] = "applet";
inline constexpr const char kContainmentProperty[] = "containment";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasmaquick/appletcontext.cpp";
}

} // namespace pbsd::kde::frameworks::plasma::plasmaquick_appletcontext
