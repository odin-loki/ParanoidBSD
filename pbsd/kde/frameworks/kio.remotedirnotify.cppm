export module pbsd.kde.kio.remotedirnotify;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (remotedirnotify.cpp).
/// Upstream: kde/frameworks/kio/src/kioworkers/remote/kdedmodule/remotedirnotify.cpp
export namespace pbsd::kde::frameworks::kio::remotedirnotify {

inline constexpr const char kModuleName[] = "remotedirnotify";
inline constexpr const char kDbusService[] = "org.kde.remotedirnotify";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/kioworkers/remote/kdedmodule/remotedirnotify.cpp";
}

} // namespace pbsd::kde::frameworks::kio::remotedirnotify
