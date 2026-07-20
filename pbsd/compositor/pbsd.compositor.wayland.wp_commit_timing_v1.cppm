module;

export module pbsd.compositor.wayland.wp_commit_timing_v1;

import pbsd.core;

/// Burst 16 wave 2 — wp-commit-timing-v1.xml (native compositor).
export namespace pbsd::compositor::wayland::wp_commit_timing_v1 {

inline constexpr const char kInterface[] = "wp_commit_timing_manager_v1";
inline constexpr int kVersion = 1;

enum class Request : unsigned char {
    Destroy = 0,
    GetTiming = 1,
};

[[nodiscard]] inline bool is_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(Request::GetTiming);
}

} // namespace pbsd::compositor::wayland::wp_commit_timing_v1
