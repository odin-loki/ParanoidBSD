export module pbsd.kde.frameworks.kcoreaddons.kmemoryinfo;

import pbsd.core;

/// Wave 3 — memory info page size (from kmemoryinfo.cpp).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kmemoryinfo.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::kmemoryinfo {

inline constexpr unsigned kPageSize = 4096;

[[nodiscard]] inline unsigned pages_for_bytes(unsigned long long bytes) noexcept {
    return static_cast<unsigned>((bytes + kPageSize - 1) / kPageSize);
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kmemoryinfo.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::kmemoryinfo
