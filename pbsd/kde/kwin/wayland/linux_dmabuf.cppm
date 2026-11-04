export module pbsd.kde.kwin.wayland.linux_dmabuf;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 18 — zwp_linux_dmabuf_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/linuxdmabuf_v1.cpp
export namespace pbsd::kde::kwin::wayland::linux_dmabuf {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_LINUX_DMABUF",
    "kwin_linux_dmabuf",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/linuxdmabuf_v1.cpp",
};

inline constexpr const char kManagerInterface[] = "zwp_linux_dmabuf_v1";
inline constexpr const char kParamsInterface[] = "zwp_linux_buffer_params_v1";
inline constexpr int kVersion = 4;

enum class TrancheFlag : unsigned char {
    Scanout = 1,
    Render = 2,
};

[[nodiscard]] inline Status validate_tranche(unsigned flags) noexcept {
    if (flags == 0 || flags > 3) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/linuxdmabuf_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::linux_dmabuf
