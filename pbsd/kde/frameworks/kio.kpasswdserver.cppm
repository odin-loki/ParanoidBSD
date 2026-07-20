export module pbsd.kde.frameworks.kio.kpasswdserver;

import pbsd.core;

/// Wave 3 — kpasswdserver KDED module identity.
/// Upstream: kde/frameworks/kio/src/kpasswdserver/kiod_kpasswdserver.cpp
export namespace pbsd::kde::frameworks::kio::kpasswdserver {

inline constexpr const char kModuleName[] = "kpasswdserver";
inline constexpr const char kDbusService[] = "org.kde.kpasswdserver";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/kpasswdserver/kiod_kpasswdserver.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kpasswdserver
