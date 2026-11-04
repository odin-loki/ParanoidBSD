export module pbsd.kde.frameworks.kio.copyjob;

import pbsd.core;

/// Wave 3 pass 5 — KIO copy job operation flags.
/// Upstream: kde/frameworks/kio/src/core/copyjob.cpp
export namespace pbsd::kde::frameworks::kio::copyjob {

    enum class Operation : unsigned { Copy = 0x01, Move = 0x02, Link = 0x04 };
    inline constexpr unsigned kDefaultPermissions = 0644;
    inline constexpr unsigned kMaxConcurrentFiles = 16;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/copyjob.cpp";
}

} // namespace pbsd::kde::frameworks::kio::copyjob
