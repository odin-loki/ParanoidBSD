module;
#include <cstdint>

export module pbsd.kernel.resource;

import pbsd.core;

/// Freestanding port of `kern_resource.c` constants + cap-mode priority policy.
export namespace pbsd::kernel::resource {

// resource.h — getpriority / setpriority
inline constexpr int kPrioMin      = -20;
inline constexpr int kPrioMax      = 20;
inline constexpr int kPrioProcess  = 0;
inline constexpr int kPrioPgrp     = 1;
inline constexpr int kPrioUser     = 2;

// resource.h — rlimit identifiers
inline constexpr int kRlimitCpu      = 0;
inline constexpr int kRlimitFsize    = 1;
inline constexpr int kRlimitData     = 2;
inline constexpr int kRlimitStack    = 3;
inline constexpr int kRlimitCore     = 4;
inline constexpr int kRlimitRss      = 5;
inline constexpr int kRlimitMemlock  = 6;
inline constexpr int kRlimitNproc    = 7;
inline constexpr int kRlimitNofile   = 8;
inline constexpr int kRlimitSbsize   = 9;
inline constexpr int kRlimitVmem     = 10;
inline constexpr int kRlimitNpts     = 11;
inline constexpr int kRlimitSwap     = 12;
inline constexpr int kRlimitKqueues  = 13;
inline constexpr int kRlimitUmtxp    = 14;
inline constexpr int kRlimitPipebuf  = 15;
inline constexpr int kRlimNlimits    = 16;

struct Rlimit {
    std::uint64_t cur{};
    std::uint64_t max{};
};

/// `kern_setpriority`: clamp nice into [PRIO_MIN, PRIO_MAX].
[[nodiscard]] constexpr int clamp_nice(int n) noexcept {
    if (n > kPrioMax) {
        return kPrioMax;
    }
    if (n < kPrioMin) {
        return kPrioMin;
    }
    return n;
}

/// `kern_getpriority` / `kern_setpriority` — IN_CAPABILITY_MODE restrictions.
[[nodiscard]] inline Status cap_mode_priority_allowed(int which, int who,
                                                      int self_pid,
                                                      bool in_cap_mode) noexcept {
    if (!in_cap_mode) {
        return Status::Ok;
    }
    if (which != kPrioProcess) {
        return Status::Denied; // ECAPMODE
    }
    if (who != 0 && who != self_pid) {
        return Status::Denied;
    }
    return Status::Ok;
}

/// Validate setpriority argument range (EINVAL → Invalid).
[[nodiscard]] constexpr Status validate_priority_arg(int prio) noexcept {
    if (prio > kPrioMax || prio < kPrioMin) {
        return Status::Invalid;
    }
    return Status::Ok;
}

/// Narrow rlimit soft value against hard ceiling (`kern_setrlimit` subset).
[[nodiscard]] constexpr Status limit_rlimit(Rlimit& lim, std::uint64_t new_cur) noexcept {
    if (new_cur > lim.max) {
        return Status::Denied;
    }
    lim.cur = new_cur;
    return Status::Ok;
}

struct RlimitName {
    int         id;
    const char* name;
};

inline constexpr RlimitName kRlimitTable[] = {
    {kRlimitCpu, "cpu"},       {kRlimitFsize, "fsize"}, {kRlimitData, "data"},
    {kRlimitStack, "stack"},   {kRlimitCore, "core"},   {kRlimitRss, "rss"},
    {kRlimitMemlock, "memlock"}, {kRlimitNproc, "nproc"}, {kRlimitNofile, "nofile"},
    {kRlimitSbsize, "sbsize"}, {kRlimitVmem, "vmem"},   {kRlimitNpts, "npts"},
    {kRlimitSwap, "swap"},     {kRlimitKqueues, "kqueues"}, {kRlimitUmtxp, "umtxp"},
    {kRlimitPipebuf, "pipebuf"},
};

[[nodiscard]] inline unsigned rlimit_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kRlimitTable) / sizeof(kRlimitTable[0]));
}

[[nodiscard]] constexpr Status validate_rlimit_id(int which) noexcept {
    if (which < 0 || which >= kRlimNlimits) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline Result<const char*> rlimit_name(int which) noexcept {
    if (validate_rlimit_id(which) != Status::Ok) {
        return result_err<const char*>(Status::Invalid);
    }
    for (const auto& e : kRlimitTable) {
        if (e.id == which) {
            return result_ok(e.name);
        }
    }
    return result_err<const char*>(Status::NotFound);
}

} // namespace pbsd::kernel::resource
