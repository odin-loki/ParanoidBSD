module;

export module pbsd.compositor.wayland.linux_dmabuf;

import pbsd.core;

/// Burst 15 — linux-dmabuf-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::linux_dmabuf {

inline constexpr const char kInterface[] = "zwp_linux_dmabuf_v1";
inline constexpr int kVersion = 4;

enum class Request : unsigned char {
    Destroy = 0,
    CreateParams = 1,
    GetDefaultFeedback = 2,
    GetSurfaceFeedback = 3,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetSurfaceFeedback);
}

} // namespace pbsd::compositor::wayland::linux_dmabuf
