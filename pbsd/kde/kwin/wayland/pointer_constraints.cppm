export module pbsd.kde.kwin.wayland.pointer_constraints;

import pbsd.core;
import pbsd.kde.kwin.logging.common;

/// Burst 17 — zwp_pointer_constraints_v1 protocol glue.
/// Upstream: kde/kwin/src/wayland/pointerconstraints_v1.cpp
export namespace pbsd::kde::kwin::wayland::pointer_constraints {

inline constexpr ::pbsd::kde::kwin::logging::Category kCategory{
    "KWIN_POINTER_CONSTRAINTS",
    "kwin_pointer_constraints",
    ::pbsd::kde::kwin::logging::kQtWarningMsg,
    "kde/kwin/src/wayland/pointerconstraints_v1.cpp",
};

inline constexpr const char kManagerInterface[] = "zwp_pointer_constraints_v1";
inline constexpr const char kLockedPointerInterface[] = "zwp_locked_pointer_v1";
inline constexpr int kVersion = 1;

enum class Lifetime : unsigned char {
    Oneshot = 1,
    Persistent = 2,
};

[[nodiscard]] inline Status validate_lifetime(unsigned lifetime) noexcept {
    if (lifetime == 0 || lifetime > static_cast<unsigned>(Lifetime::Persistent)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/pointerconstraints_v1.cpp";
}

} // namespace pbsd::kde::kwin::wayland::pointer_constraints
