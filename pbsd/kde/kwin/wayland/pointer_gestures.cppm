export module pbsd.kde.kwin.wayland.pointer_gestures;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 18 — zwp_pointer_gestures_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/pointergestures_v1.cpp
export namespace pbsd::kde::kwin::wayland::pointer_gestures {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_POINTER_GESTURES",
    "kwin_pointer_gestures",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/pointergestures_v1.cpp",
};

inline constexpr const char kManagerInterface[] = "zwp_pointer_gestures_v1";
inline constexpr const char kSwipeInterface[] = "zwp_pointer_gesture_swipe_v1";
inline constexpr const char kPinchInterface[] = "zwp_pointer_gesture_pinch_v1";
inline constexpr int kVersion = 1;

enum class GestureType : unsigned char {
    Swipe = 0,
    Pinch = 1,
};

[[nodiscard]] inline Status validate_gesture(unsigned g) noexcept {
    return g <= static_cast<unsigned>(GestureType::Pinch) ? Status::Ok : Status::Invalid;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/pointergestures_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::pointer_gestures
