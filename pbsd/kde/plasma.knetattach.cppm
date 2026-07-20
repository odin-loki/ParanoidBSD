export module pbsd.kde.plasma.knetattach;

import pbsd.core;

/// Wave 3 pass 4 — Network attach wizard id.
/// Upstream: kde/plasma-desktop/knetattach/main.cpp
export namespace pbsd::kde::plasma::knetattach {

    inline constexpr const char kAppId[] = "knetattach";
    inline constexpr const char kProtocolKey[] = "protocol";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/knetattach/main.cpp";
}

} // namespace pbsd::kde::plasma::knetattach
