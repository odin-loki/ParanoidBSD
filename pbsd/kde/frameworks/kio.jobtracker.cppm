export module pbsd.kde.frameworks.kio.jobtracker;

import pbsd.core;

/// Wave 3 pass 3 — KIO job tracker interface.
/// Upstream: kde/frameworks/kio/src/core/jobtracker.cpp
export namespace pbsd::kde::frameworks::kio::jobtracker {

    inline constexpr const char kInterfaceId[] = "org.kde.kio.jobtracker";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/jobtracker.cpp";
}

} // namespace pbsd::kde::frameworks::kio::jobtracker
