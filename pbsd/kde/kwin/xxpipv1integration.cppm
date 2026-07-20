export module pbsd.kde.kwin.xxpipv1integration;

import pbsd.core;

/// Wave 3 pass 4 — Xwayland XPresent integration stub.
/// Upstream: kde/kwin/src/xxpipv1integration.cpp
export namespace pbsd::kde::kwin::xxpipv1integration {

    inline constexpr const char kProtocolName[] = "XPresent";
    inline constexpr unsigned kVersion = 1;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/xxpipv1integration.cpp";
}

} // namespace pbsd::kde::kwin::xxpipv1integration
