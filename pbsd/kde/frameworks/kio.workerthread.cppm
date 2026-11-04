export module pbsd.kde.frameworks.kio.workerthread;

import pbsd.core;

/// Wave 3 — hand port constants (workerthread.cpp).
/// Upstream: kde/frameworks/kio/src/core/workerthread.cpp
export namespace pbsd::kde::frameworks::kio::workerthread {

inline constexpr unsigned kWorkerStackSize = 65536;
inline constexpr const char kWorkerThreadName[] = "KIO::Worker";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kio/src/core/workerthread.cpp";
}

} // namespace pbsd::kde::frameworks::kio::workerthread
