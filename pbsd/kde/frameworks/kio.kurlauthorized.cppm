export module pbsd.kde.frameworks.kio.kurlauthorized;

import pbsd.core;

/// Wave 3 — hand port constants (kurlauthorized.cpp).
/// Upstream: kde/frameworks/kio/src/core/kurlauthorized.cpp
export namespace pbsd::kde::frameworks::kio::kurlauthorized {

inline constexpr const char kOpenFile[] = "open";
inline constexpr const char kOpenUrl[] = "openUrl";
inline constexpr const char kListDir[] = "list";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/kurlauthorized.cpp";
}

} // namespace pbsd::kde::frameworks::kio::kurlauthorized
