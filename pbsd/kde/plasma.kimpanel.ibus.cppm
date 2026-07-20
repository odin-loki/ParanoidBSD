export module pbsd.kde.plasma.kimpanel.ibus;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (main.cpp).
/// Upstream: kde/plasma-desktop/applets/kimpanel/backend/ibus/ibus15/main.cpp
export namespace pbsd::kde::plasma::kimpanel::ibus {

inline constexpr const char kIbusService[] = "org.freedesktop.IBus";
inline constexpr const char kIbusPath[] = "/org/freedesktop/IBus";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/applets/kimpanel/backend/ibus/ibus15/main.cpp";
}

} // namespace pbsd::kde::plasma::kimpanel::ibus
