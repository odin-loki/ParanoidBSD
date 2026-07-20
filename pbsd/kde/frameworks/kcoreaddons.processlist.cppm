export module pbsd.kde.frameworks.kcoreaddons.processlist;

import pbsd.core;
import pbsd.kde.frameworks.kcoreaddons.procstat;

/// Wave 3 — process info façade (from KProcessList + procstat path).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kprocesslist.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::processlist {

using ProcessInfo = procstat::ProcessInfo;
using ProcessList = procstat::ProcessList;
inline constexpr unsigned kMaxProcesses = procstat::kMaxProcesses;

[[nodiscard]] inline Status process_info_list(ProcessList& out) noexcept {
    return procstat::process_list(out);
}

[[nodiscard]] inline Result<ProcessInfo> process_info(int pid) noexcept {
    return procstat::process_info(pid);
}

[[nodiscard]] inline bool is_valid(const ProcessInfo& info) noexcept {
    return info.pid > 0;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kprocesslist.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::processlist
