module;

export module pbsd.compositor.wayland.viewporter;

import pbsd.core;

/// Burst 13 — wp_viewporter (native compositor).
export namespace pbsd::compositor::wayland::viewporter {

inline constexpr const char kInterface[] = "wp_viewporter";
inline constexpr int kVersion = 1;

enum class ViewporterRequest : unsigned char {
    Destroy = 0,
    GetViewport = 1,
};

enum class ViewportRequest : unsigned char {
    Destroy = 0,
    SetSource = 1,
    SetDestination = 2,
};

[[nodiscard]] inline bool is_viewporter_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ViewporterRequest::GetViewport);
}

[[nodiscard]] inline bool is_viewport_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ViewportRequest::SetDestination);
}

[[nodiscard]] inline Status validate_source(double w, double h) noexcept {
    if (w < 0.0 || h < 0.0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Status validate_destination(int w, int h) noexcept {
    if (w < 0 || h < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::compositor::wayland::viewporter
