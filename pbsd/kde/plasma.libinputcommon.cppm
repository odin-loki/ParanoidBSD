export module pbsd.kde.plasma.libinputcommon;

import pbsd.core;

/// Wave 3 pass 3 — Touchpad libinput common constants.
/// Upstream: kde/plasma-desktop/kcms/touchpad/backends/libinputcommon.cpp
export namespace pbsd::kde::plasma::libinputcommon {

    inline constexpr const char kBackendName[] = "libinput";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/touchpad/backends/libinputcommon.cpp";
}

} // namespace pbsd::kde::plasma::libinputcommon
