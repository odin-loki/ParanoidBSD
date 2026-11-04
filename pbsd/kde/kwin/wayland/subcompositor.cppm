export module pbsd.kde.kwin.wayland.subcompositor;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 11 — wl_subcompositor / wl_subsurface protocol glue.
/// Upstream: kde/kwin/src/wayland/subcompositor.cpp
export namespace pbsd::kde::kwin::wayland::subcompositor {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_SUBCOMPOSITOR",
    "kwin_subcompositor",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/subcompositor.cpp",
};

inline constexpr const char kInterface[] = "wl_subcompositor";
inline constexpr int kVersion = 1;

enum class SubcompositorRequest : unsigned char {
    Destroy       = 0,
    GetSubsurface = 1,
};

enum class SubsurfaceRequest : unsigned char {
    Destroy              = 0,
    SetPosition          = 1,
    PlaceAbove           = 2,
    PlaceBelow           = 3,
    SetSync              = 4,
    SetDesync            = 5,
};

[[nodiscard]] inline bool is_subcompositor_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(SubcompositorRequest::GetSubsurface);
}

[[nodiscard]] inline bool is_subsurface_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(SubsurfaceRequest::SetDesync);
}

[[nodiscard]] inline Status validate_position(int x, int y) noexcept {
    if (x < -32768 || x > 32767 || y < -32768 || y > 32767) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/subcompositor.cpp";
}

} // namespace pbsd::kde::kwin::wayland::subcompositor
