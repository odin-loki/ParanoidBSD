module;

export module pbsd.compositor.wayland.content_type;

import pbsd.core;

/// Burst 12 — wp_content_type_manager_v1 (native compositor).
export namespace pbsd::compositor::wayland::content_type {

inline constexpr const char kManagerInterface[] = "wp_content_type_manager_v1";
inline constexpr const char kSurfaceInterface[] = "wp_content_type_v1";
inline constexpr int kVersion = 1;

enum class ContentHint : unsigned char {
    None  = 0,
    Photo = 1,
    Video = 2,
    Game  = 3,
    Text  = 4,
};

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    Get     = 1,
};

enum class SurfaceRequest : unsigned char {
    Destroy = 0,
    Set     = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Get);
}

[[nodiscard]] inline bool is_surface_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(SurfaceRequest::Set);
}

[[nodiscard]] inline Status validate_hint(unsigned hint) noexcept {
    if (hint > static_cast<unsigned>(ContentHint::Text)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::content_type
