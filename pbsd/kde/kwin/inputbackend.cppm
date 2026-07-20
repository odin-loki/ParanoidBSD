export module pbsd.kde.kwin.inputbackend;

import pbsd.core;

/// Wave 3 pass 4 — Input backend name constants.
/// Upstream: kde/kwin/src/core/inputbackend.cpp
export namespace pbsd::kde::kwin::inputbackend {

    inline constexpr const char kLibinputBackend[] = "libinput";
    inline constexpr const char kX11Backend[] = "x11";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/core/inputbackend.cpp";
}

} // namespace pbsd::kde::kwin::inputbackend
