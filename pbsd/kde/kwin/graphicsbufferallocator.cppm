export module pbsd.kde.kwin.graphicsbufferallocator;

import pbsd.core;

/// Wave 3 pass 3 — Graphics buffer allocator constants.
/// Upstream: kde/kwin/src/core/graphicsbufferallocator.cpp
export namespace pbsd::kde::kwin::graphicsbufferallocator {

    inline constexpr unsigned kDefaultPoolSize{16};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/core/graphicsbufferallocator.cpp";
}

} // namespace pbsd::kde::kwin::graphicsbufferallocator
