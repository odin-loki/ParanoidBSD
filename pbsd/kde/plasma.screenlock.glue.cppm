export module pbsd.kde.plasma.screenlock.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.screenlock;

/// Burst 16 — Plasma screenlock ↔ Aero glue (string constants; no layershellintegration import).
/// Upstream: kde/plasma-workspace/screenlocker/main.cpp
export namespace pbsd::kde::plasma::screenlock::glue {

inline constexpr const char kLayerShell[] = "zwlr_layer_shell_v1";
inline constexpr const char kShellSurface[] = "zwlr_layer_surface_v1";

struct ScreenlockStyle {
    const char* greeter_id{screenlock::kGreeterId};
    unsigned grace_ms{screenlock::kGraceMs};
    const char* lock_svg{aero::kLockSvg};
};

[[nodiscard]] inline ScreenlockStyle default_style() noexcept {
    return ScreenlockStyle{};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return ::pbsd::kde::plasma::screenlock::upstream_path();
}

} // namespace pbsd::kde::plasma::screenlock::glue
