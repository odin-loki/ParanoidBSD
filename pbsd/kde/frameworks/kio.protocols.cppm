export module pbsd.kde.frameworks.kio.protocols;

import pbsd.core;

/// Wave 3 pass 5 — KIO protocol scheme identifiers.
/// Upstream: kde/frameworks/kio/src/core/kprotocolmanager.cpp
export namespace pbsd::kde::frameworks::kio::protocols {

    inline constexpr const char kFile[] = "file";
    inline constexpr const char kHttp[] = "http";
    inline constexpr const char kHttps[] = "https";
    inline constexpr const char kFtp[] = "ftp";
    inline constexpr const char kTrash[] = "trash";
    inline constexpr const char kDesktop[] = "desktop";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/kprotocolmanager.cpp";
}

} // namespace pbsd::kde::frameworks::kio::protocols
