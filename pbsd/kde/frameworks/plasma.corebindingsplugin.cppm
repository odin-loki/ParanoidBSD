export module pbsd.kde.frameworks.plasma.corebindingsplugin;

import pbsd.core;

/// Wave 3 pass 4 — Plasma core QML bindings plugin.
/// Upstream: kde/frameworks/plasma-framework/src/declarativeimports/core/corebindingsplugin.cpp
export namespace pbsd::kde::frameworks::plasma::corebindingsplugin {

    inline constexpr const char kPluginId[] = "org.kde.plasma.core";
    inline constexpr const char kQmlModule[] = "org.kde.plasma.core";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/declarativeimports/core/corebindingsplugin.cpp";
}

} // namespace pbsd::kde::frameworks::plasma::corebindingsplugin
