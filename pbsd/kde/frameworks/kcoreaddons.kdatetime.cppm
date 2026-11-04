export module pbsd.kde.frameworks.kcoreaddons.kdatetime;

import pbsd.core;

/// Wave 3 pass 5 — KDateTime ISO format constants.
/// Upstream: kde/frameworks/kcoreaddons/src/lib/datetime/kdatetime.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kdatetime {

    inline constexpr const char kIsoDateFormat[] = "yyyy-MM-dd";
    inline constexpr const char kIsoTimeFormat[] = "HH:mm:ss";
    inline constexpr const char kIsoDateTimeFormat[] = "yyyy-MM-ddTHH:mm:ss";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/datetime/kdatetime.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kdatetime
