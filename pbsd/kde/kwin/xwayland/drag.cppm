export module pbsd.kde.drag;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (drag.cpp).
/// Upstream: kde/kwin/src/xwayland/drag.cpp
export namespace pbsd::kde::drag {

inline constexpr const char kXdndMimeType[] = "XdndTypeList";
inline constexpr unsigned kMaxDragTargets = 16;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/xwayland/drag.cpp";
}

} // namespace pbsd::kde::drag
