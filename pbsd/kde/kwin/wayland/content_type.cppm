export module pbsd.kde.kwin.wayland.content_type;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 12 — wp_content_type_manager_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/contenttype.cpp
export namespace pbsd::kde::kwin::wayland::content_type {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_CONTENT_TYPE",
    "kwin_content_type",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/contenttype.cpp",
};

inline constexpr const char kManagerInterface[] = "wp_content_type_manager_v1";
inline constexpr const char kSurfaceInterface[] = "wp_content_type_v1";
inline constexpr int kVersion = 1;

enum class ContentHint : unsigned char {
    None       = 0,
    Photo      = 1,
    Video      = 2,
    Game       = 3,
    Text       = 4,
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

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/contenttype.cpp";
}

} // namespace pbsd::kde::kwin::wayland::content_type
