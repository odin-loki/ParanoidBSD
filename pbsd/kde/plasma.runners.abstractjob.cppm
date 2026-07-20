export module pbsd.kde.plasma.runners.abstractjob;

import pbsd.core;

/// Wave 3 pass 4 — hand port constants (AbstractJob.cpp).
/// Upstream: kde/plasma-desktop/kcms/runners/plugininstaller/AbstractJob.cpp
export namespace pbsd::kde::plasma::runners::abstractjob {

inline constexpr const char kJobStatePending[] = "pending";
inline constexpr const char kJobStateFinished[] = "finished";

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/plasma-desktop/kcms/runners/plugininstaller/AbstractJob.cpp";
}

} // namespace pbsd::kde::plasma::runners::abstractjob
