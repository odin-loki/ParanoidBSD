export module pbsd.kde.frameworks.kio.kpasswdserverloop;

import pbsd.core;

/// Wave 3 — hand port constants (kpasswdserverloop.cpp).
/// Upstream: kde/frameworks/kio/src/core/kpasswdserverloop.cpp
export namespace pbsd::kde::frameworks::kio::kpasswdserverloop {

inline constexpr const char kLoopService[] = "org.kde.kpasswdserverloop";
inline constexpr unsigned kMaxRetries = 3;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/kpasswdserverloop.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kpasswdserverloop
