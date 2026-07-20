export module pbsd.kde.kwin.wayland.fractional_scale;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 11 — wp_fractional_scale_v1 HiDPI scale protocol glue.
/// Upstream: kde/kwin/src/wayland/fractionalscale.cpp
export namespace pbsd::kde::kwin::wayland::fractional_scale {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_FRACTIONAL_SCALE",
    "kwin_fractional_scale",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/fractionalscale.cpp",
};

inline constexpr const char kManagerInterface[] = "wp_fractional_scale_manager_v1";
inline constexpr const char kScaleInterface[] = "wp_fractional_scale_v1";
inline constexpr int kVersion = 1;

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
    if (num < 120 || num > 480) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline float scale_from_numerator(unsigned num) noexcept {
    return static_cast<float>(num) / 120.0f;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/fractionalscale.cpp";
}

} // namespace pbsd::kde::kwin::wayland::fractional_scale
