export module pbsd.kde.abstract_drop_handler;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (abstract_drop_handler.cpp).
/// Upstream: kde/kwin/src/wayland/abstract_drop_handler.cpp
export namespace pbsd::kde::abstract_drop_handler {

inline constexpr const char kMimeTextPlain[] = "text/plain";
inline constexpr const char kMimeUriList[] = "text/uri-list";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/wayland/abstract_drop_handler.cpp";
}

} // namespace pbsd::kde::abstract_drop_handler
