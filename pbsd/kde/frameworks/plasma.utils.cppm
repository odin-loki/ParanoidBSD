export module pbsd.kde.frameworks.plasma.utils;

import pbsd.core;

/// Wave 3 pass 3 — PlasmaQuick utils constants.
/// Upstream: kde/frameworks/plasma-framework/src/plasmaquick/utils.cpp
export namespace pbsd::kde::frameworks::plasma::utils {

    inline constexpr const char kUtilsUri[] = "org.kde.plasma.utils";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/plasma-framework/src/plasmaquick/utils.cpp";
}

} // namespace pbsd::kde::frameworks::plasma::utils
