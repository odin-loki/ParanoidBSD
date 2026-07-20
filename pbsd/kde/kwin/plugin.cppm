export module pbsd.kde.kwin.plugin;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Wave 3 pass 3 — KWin PluginFactory stub.
/// Upstream: kde/kwin/src/plugin.cpp
export namespace pbsd::kde::kwin::plugin {

    inline constexpr const char kFactoryId[] = "org.kde.kwin.plugin";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugin.cpp";
}

} // namespace pbsd::kde::kwin::plugin
