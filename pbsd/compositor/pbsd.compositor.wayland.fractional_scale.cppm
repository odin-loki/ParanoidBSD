module;

export module pbsd.compositor.wayland.fractional_scale;

import pbsd.core;

/// Burst 12 — wp_fractional_scale_v1 HiDPI scale (native compositor).
export namespace pbsd::compositor::wayland::fractional_scale {

inline constexpr const char kManagerInterface[] = "wp_fractional_scale_manager_v1";
inline constexpr const char kScaleInterface[] = "wp_fractional_scale_v1";
inline constexpr int kVersion = 1;
inline constexpr unsigned kScaleBase = 120;

enum class ManagerRequest : unsigned char {
    Destroy = 0,
    Get     = 1,
};

enum class ScaleEvent : unsigned char {
    PreferredScale = 0,
};

[[nodiscard]] inline bool is_manager_request(unsigned opcode) noexcept {
    return opcode <= static_cast<unsigned>(ManagerRequest::Get);
}

[[nodiscard]] inline Status validate_scale_numerator(unsigned num) noexcept {
    if (num < kScaleBase || num > 480) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline float scale_from_numerator(unsigned num) noexcept {
    return static_cast<float>(num) / static_cast<float>(kScaleBase);
}

} // namespace pbsd::compositor::wayland::fractional_scale
