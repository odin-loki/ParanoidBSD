module;

#include <cstdlib>
#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.ksandbox;

import pbsd.core;

/// Wave 3 — sandbox detection (Flatpak/Snap paths from KSandbox).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/ksandbox.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::ksandbox {

inline constexpr const char kFlatpakInfoPath[] = "/.flatpak-info";
inline constexpr const char kSnapEnvVar[] = "SNAP";
inline constexpr const char kFlatpakTalkToken[] = "\norg.freedesktop.Flatpak=talk\n";

[[nodiscard]] inline bool is_flatpak() noexcept {
#if defined(__FreeBSD__)
    return false;
#else
    return false; // hosted Linux path checks deferred
#endif
}

[[nodiscard]] inline bool is_snap() noexcept {
    const char* snap = std::getenv(kSnapEnvVar);
    return snap != nullptr && snap[0] != '\0';
}

[[nodiscard]] inline bool is_inside() noexcept {
    return is_flatpak() || is_snap();
}

[[nodiscard]] inline bool has_flatpak_spawn_privileges() noexcept {
    return false; // requires hosted file read
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/ksandbox.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::ksandbox
