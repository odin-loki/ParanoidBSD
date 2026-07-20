export module pbsd.kde.frameworks.kio.joburlcache;

import pbsd.core;

/// Wave 3 — hand port constants (joburlcache.cpp).
/// Upstream: kde/frameworks/kio/src/widgets/joburlcache.cpp
export namespace pbsd::kde::frameworks::kio::joburlcache {

inline constexpr unsigned kMaxCachedUrls = 128;
inline constexpr unsigned kMaxUrlLen = 2048;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/widgets/joburlcache.cpp";
}

} // namespace pbsd::kde::frameworks::kio::joburlcache
