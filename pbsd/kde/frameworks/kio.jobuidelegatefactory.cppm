export module pbsd.kde.frameworks.kio.jobuidelegatefactory;

import pbsd.core;

/// Wave 3 — hand port constants (jobuidelegatefactory.cpp).
/// Upstream: kde/frameworks/kio/src/core/jobuidelegatefactory.cpp
export namespace pbsd::kde::frameworks::kio::jobuidelegatefactory {

inline constexpr const char kFactoryIid[] = "org.kde.kio.jobuidelegatefactory/1.0";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/jobuidelegatefactory.cpp";
}

} // namespace pbsd::kde::frameworks::kio::jobuidelegatefactory
