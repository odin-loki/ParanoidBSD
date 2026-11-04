module;

export module pbsd.compositor.wayland.alpha_modifier;

import pbsd.core;

/// Burst 12 — wp_alpha_modifier_v1 (native compositor).
export namespace pbsd::compositor::wayland::alpha_modifier {

inline constexpr const char kManagerInterface[] = "wp_alpha_modifier_v1";
inline constexpr const char kSurfaceInterface[] = "wp_alpha_modifier_surface_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    Get     = 1,
};

enum class SurfaceRequest : unsigned char {
    Destroy       = 0,
    SetFactor     = 1,
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

} // namespace pbsd::compositor::wayland::alpha_modifier
