export module pbsd.kde.frameworks.kio.kurlnavigatorpathselectoreventfilter;

import pbsd.core;

/// Wave 3 pass 4 — URL navigator path selector filter.
/// Upstream: kde/frameworks/kio/src/filewidgets/kurlnavigatorpathselectoreventfilter.cpp
export namespace pbsd::kde::frameworks::kio::kurlnavigatorpathselectoreventfilter {

    inline constexpr unsigned kMaxPathLen = 4096;
    inline constexpr const char kHomePath[] = "~";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/filewidgets/kurlnavigatorpathselectoreventfilter.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kurlnavigatorpathselectoreventfilter
