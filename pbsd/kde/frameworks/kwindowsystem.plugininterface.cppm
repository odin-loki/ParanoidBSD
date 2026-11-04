export module pbsd.kde.frameworks.kwindowsystem.plugininterface;

import pbsd.core;

/// Wave 3 pass 3 — KWindowSystem plugin interface.
/// Upstream: kde/frameworks/kwindowsystem/src/kwindowsystemplugininterface.cpp
export namespace pbsd::kde::frameworks::kwindowsystem::plugininterface {

    inline constexpr const char kInterfaceId[] = "org.kde.kwindowsystem.plugin";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwindowsystem/src/kwindowsystemplugininterface.cpp";
}

} // namespace pbsd::kde::frameworks::kwindowsystem::plugininterface
