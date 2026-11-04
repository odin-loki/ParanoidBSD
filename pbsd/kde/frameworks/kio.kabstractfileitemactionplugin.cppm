export module pbsd.kde.frameworks.kio.kabstractfileitemactionplugin;

import pbsd.core;

/// Wave 3 pass 3 — Abstract file item action plugin.
/// Upstream: kde/frameworks/kio/src/widgets/kabstractfileitemactionplugin.cpp
export namespace pbsd::kde::frameworks::kio::kabstractfileitemactionplugin {

    inline constexpr const char kPluginId[] = "org.kde.kio.fileitemaction";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/widgets/kabstractfileitemactionplugin.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kabstractfileitemactionplugin
