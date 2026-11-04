export module pbsd.kde.kio.remotedirnotifymodule;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (remotedirnotifymodule.cpp).
/// Upstream: kde/frameworks/kio/src/kioworkers/remote/kdedmodule/remotedirnotifymodule.cpp
export namespace pbsd::kde::frameworks::kio::remotedirnotifymodule {

inline constexpr const char kModuleName[] = "remotedirnotifymodule";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/kioworkers/remote/kdedmodule/remotedirnotifymodule.cpp";
}

} // namespace pbsd::kde::frameworks::kio::remotedirnotifymodule
