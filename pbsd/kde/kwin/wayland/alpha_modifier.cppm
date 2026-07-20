export module pbsd.kde.kwin.wayland.alpha_modifier;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 12 — wp_alpha_modifier_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/alphamodifier.cpp
export namespace pbsd::kde::kwin::wayland::alpha_modifier {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_ALPHA_MODIFIER",
    "kwin_alpha_modifier",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/alphamodifier.cpp",
};

inline constexpr const char kManagerInterface[] = "wp_alpha_modifier_v1";
inline constexpr const char kSurfaceInterface[] = "wp_alpha_modifier_surface_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    Get     = 1,
};

enum class SurfaceRequest : unsigned char {
    Destroy     = 0,
    SetFactor   = 1,
    SetMultiplier = 2,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Get);
}

[[nodiscard]] inline bool is_surface_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(SurfaceRequest::SetMultiplier);
}

[[nodiscard]] inline Status validate_factor(unsigned factor) noexcept {
    if (factor > 0xFFFF) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline float factor_to_opacity(unsigned factor) noexcept {
    return static_cast<float>(factor) / 65535.0f;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/alphamodifier.cpp";
}

} // namespace pbsd::kde::kwin::wayland::alpha_modifier
