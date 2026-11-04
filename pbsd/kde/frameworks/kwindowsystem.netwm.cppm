export module pbsd.kde.frameworks.kwindowsystem.netwm;

import pbsd.core;

/// Wave 3 pass 5 — NetWM window state bit flags.
/// Upstream: kde/frameworks/kwindowsystem/src/netwm_def.h
export namespace pbsd::kde::frameworks::kwindowsystem::netwm {

    enum class State : unsigned {
        Modal = 1u << 0,
        Sticky = 1u << 1,
        MaxVert = 1u << 2,
        MaxHoriz = 1u << 3,
        Max = MaxVert | MaxHoriz,
        Shaded = 1u << 4,
        SkipTaskbar = 1u << 5,
        KeepAbove = 1u << 6,
        KeepBelow = 1u << 7,
        Fullscreen = 1u << 11,
    };

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kwindowsystem/src/netwm_def.h";
}

} // namespace pbsd::kde::frameworks::kwindowsystem::netwm
