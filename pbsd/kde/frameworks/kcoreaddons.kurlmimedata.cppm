export module pbsd.kde.frameworks.kcoreaddons.kurlmimedata;

import pbsd.core;

/// Wave 3 — hand port constants (kurlmimedata.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/kurlmimedata.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kurlmimedata {

inline constexpr const char kUrlsMime[] = "text/uri-list";
inline constexpr const char kMostLocalUrlMime[] = "text/x-kde-urls";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/kurlmimedata.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kurlmimedata
