module;

#include <cstdlib>
#include <cstring>

export module pbsd.kde.frameworks.kcoreaddons.klistopenfiles_unix;

import pbsd.core;
import pbsd.kde.frameworks.kcoreaddons.processlist;

/// Wave 3 — lsof + procstat open-files job (FreeBSD path from klistopenfilesjob_unix).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/klistopenfilesjob_unix.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::klistopenfiles {

inline constexpr unsigned kMaxPids = 256;
inline constexpr const char kLsofExecutable[] = "lsof";
inline constexpr const char kLsofArgs[] = "-t +d";

struct PidList {
    int pids[kMaxPids]{};
    unsigned count{0};
};

[[nodiscard]] inline Status parse_lsof_output(const char* output, PidList& out) noexcept {
    if (output == nullptr) {
        return Status::Invalid;
    }
    out.count = 0;
    const char* cursor = output;
    while (*cursor != '\0' && out.count < kMaxPids) {
        while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') {
            ++cursor;
        }
        if (*cursor == '\0') {
            break;
        }
        int pid = 0;
        while (*cursor >= '0' && *cursor <= '9') {
            pid = pid * 10 + (*cursor - '0');
            ++cursor;
        }
        if (pid > 0) {
            out.pids[out.count++] = pid;
        }
        while (*cursor != '\0' && *cursor != ' ' && *cursor != '\t' && *cursor != '\n') {
            ++cursor;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline Status resolve_processes(const PidList& pids,
                                              processlist::ProcessList& out) noexcept {
    out.count = 0;
    for (unsigned i = 0; i < pids.count && out.count < processlist::kMaxProcesses;
         ++i) {
        const auto result = processlist::process_info(pids.pids[i]);
        if (result.status == Status::Ok && processlist::is_valid(result.value)) {
            out.entries[out.count++] = result.value;
        }
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* find_lsof_in_path() noexcept {
    const char* path = std::getenv("PATH");
    if (path == nullptr) {
        return nullptr;
    }
    return kLsofExecutable; // hosted job resolves via PATH at runtime
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/klistopenfilesjob_unix.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::klistopenfiles
