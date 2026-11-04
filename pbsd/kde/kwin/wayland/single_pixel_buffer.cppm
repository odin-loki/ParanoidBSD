export module pbsd.kde.kwin.wayland.single_pixel_buffer;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 18 — wp_single_pixel_buffer_manager_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/singlepixelbuffer_v1.cpp
export namespace pbsd::kde::kwin::wayland::single_pixel_buffer {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_SINGLE_PIXEL_BUFFER",
    "kwin_single_pixel_buffer",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/singlepixelbuffer_v1.cpp",
};

inline constexpr const char kManagerInterface[] = "wp_single_pixel_buffer_manager_v1";
inline constexpr const char kBufferInterface[] = "wp_single_pixel_buffer_v1";
inline constexpr int kVersion = 1;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    CreateU32RgbaBuffer = 1,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::CreateU32RgbaBuffer);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/singlepixelbuffer_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::single_pixel_buffer
