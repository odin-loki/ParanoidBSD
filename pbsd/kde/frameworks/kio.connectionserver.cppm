export module pbsd.kde.frameworks.kio.connectionserver;

import pbsd.core;

/// Wave 3 — hand port constants (connectionserver.cpp).
/// Upstream: kde/frameworks/kio/src/core/connectionserver.cpp
export namespace pbsd::kde::frameworks::kio::connectionserver {

inline constexpr const char kWorkerSocketEnv[] = "KDE_FORK_SLAVES";
inline constexpr unsigned kMaxPendingConnections = 32;

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/connectionserver.cpp";
}

} // namespace pbsd::kde::frameworks::kio::connectionserver
