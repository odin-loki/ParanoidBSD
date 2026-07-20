module;

#if defined(__FreeBSD__)
#include <cstring>

#include <sys/types.h>
#include <sys/user.h>
#include <sys/queue.h>
#include <libprocstat.h>
#endif

export module pbsd.kde.frameworks.kcoreaddons.procstat;

import pbsd.core;

/// Wave 3 — FreeBSD procstat process enumeration (from KCoreAddons procstat path).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/kprocesslist_unix_procstat.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::procstat {

inline constexpr unsigned kMaxComm = 16;
inline constexpr unsigned kMaxUser = 32;
inline constexpr unsigned kMaxProcesses = 4096;

struct ProcessInfo {
    int pid{0};
    char comm[kMaxComm]{};
    char user[kMaxUser]{};
};

struct ProcessList {
    ProcessInfo entries[kMaxProcesses]{};
    unsigned count{0};
};

namespace detail {

#if defined(__FreeBSD__)

struct ProcStatHandle {
    procstat* pstat{nullptr};

    ProcStatHandle() noexcept { pstat = procstat_open_sysctl(); }
    ~ProcStatHandle() noexcept {
        if (pstat != nullptr) {
            procstat_close(pstat);
        }
    }
    ProcStatHandle(const ProcStatHandle&) = delete;
    ProcStatHandle& operator=(const ProcStatHandle&) = delete;

    [[nodiscard]] explicit operator bool() const noexcept { return pstat != nullptr; }
};

[[nodiscard]] inline Status enumerate(ProcessList& out) noexcept {
    ProcStatHandle handle;
    if (!handle) {
        return Status::Protocol;
    }

    unsigned int proc_count = 0;
    kinfo_proc* procs = procstat_getprocs(handle.pstat, KERN_PROC_PROC, 0, &proc_count);
    if (procs == nullptr || proc_count == 0) {
        return Status::Ok;
    }

    out.count = 0;
    for (unsigned int i = 0; i < proc_count && out.count < kMaxProcesses; ++i) {
        const kinfo_proc& proc = procs[i];
        ProcessInfo& info = out.entries[out.count];
        info.pid = proc.ki_pid;

        std::memset(info.comm, 0, kMaxComm);
        std::memset(info.user, 0, kMaxUser);
        std::strncpy(info.comm, proc.ki_comm, kMaxComm - 1);
        std::strncpy(info.user, proc.ki_login, kMaxUser - 1);
        ++out.count;
    }

    procstat_freeprocs(handle.pstat, procs);
    return Status::Ok;
}

#else

[[nodiscard]] inline Status enumerate(ProcessList& out) noexcept {
    out.count = 0;
    return Status::Ok;
}

#endif

} // namespace detail

[[nodiscard]] inline Status process_list(ProcessList& out) noexcept {
    out.count = 0;
    return detail::enumerate(out);
}

[[nodiscard]] inline Result<ProcessInfo> process_info(int pid) noexcept {
    ProcessList list{};
    const Status st = process_list(list);
    if (st != Status::Ok) {
        return {st, ProcessInfo{}};
    }
    for (unsigned i = 0; i < list.count; ++i) {
        if (list.entries[i].pid == pid) {
            return {Status::Ok, list.entries[i]};
        }
    }
    return {Status::NotFound, ProcessInfo{}};
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/kprocesslist_unix_procstat.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::procstat
