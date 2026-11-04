export module pbsd.kde.frameworks.kio.statjob;

import pbsd.core;

/// Wave 3 pass 5 — KIO stat job detail level flags.
/// Upstream: kde/frameworks/kio/src/core/statjob.cpp
export namespace pbsd::kde::frameworks::kio::statjob {

    enum class Details : unsigned { None = 0, Access = 0x01, Size = 0x02, User = 0x04, Group = 0x08, Permissions = 0x10, All = 0x1f };
    inline constexpr unsigned kDefaultTimeoutMs = 30000;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/statjob.cpp";
}

} // namespace pbsd::kde::frameworks::kio::statjob
