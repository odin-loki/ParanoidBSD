export module pbsd.kde.frameworks.kio.specialjob;

import pbsd.core;

/// Wave 3 — hand port constants (specialjob.cpp).
/// Upstream: kde/frameworks/kio/src/core/specialjob.cpp
export namespace pbsd::kde::frameworks::kio::specialjob {

inline constexpr const char kSpecialProtocol[] = "special";
inline constexpr const char kAppsProtocol[] = "apps";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/specialjob.cpp";
}

} // namespace pbsd::kde::frameworks::kio::specialjob
