module;
#include <cstdint>

export module pbsd.kernel.timekeeping;

import pbsd.core;

/// Freestanding port of `sys/clock.h`, `kern_time.c` clock IDs and timespec helpers.
export namespace pbsd::kernel::timekeeping {

inline constexpr int kNsPerSec = 1000000000;

inline constexpr int kClockRealtime      = 0;
inline constexpr int kClockMonotonic     = 4;
inline constexpr int kClockProcessCputimeId = 2;
inline constexpr int kClockThreadCputimeId  = 3;
inline constexpr int kClockMonotonicRaw  = 11;
inline constexpr int kClockTai           = 16;
inline constexpr int kMaxClocks          = kClockTai + 1;

inline constexpr unsigned kCpucLockBit         = 0x80000000u;
inline constexpr unsigned kCpucLockProcessBit  = 0x40000000u;
inline constexpr unsigned kCpucLockIdMask      = ~(kCpucLockBit | kCpucLockProcessBit);

struct Clocktime {
    int  year{};
    int  mon{};
    int  day{};
    int  hour{};
    int  min{};
    int  sec{};
    int  dow{};
    long nsec{};
};

struct Timespec {
    long sec{};
    long nsec{};
};

[[nodiscard]] constexpr Status validate_clock_id(int id) noexcept {
    if (id < 0 || id >= kMaxClocks) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_timespec(const Timespec& ts) noexcept {
    if (ts.nsec < 0 || ts.nsec >= kNsPerSec) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_clocktime(const Clocktime& ct) noexcept {
    if (ct.mon < 1 || ct.mon > 12 || ct.day < 1 || ct.day > 31) {
        return Status::Invalid;
    }
    if (ct.hour < 0 || ct.hour > 23 || ct.min < 0 || ct.min > 59) {
        return Status::Invalid;
    }
    if (ct.sec < 0 || ct.sec > 60) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr int normalize_year(int year) noexcept {
    if (year >= 0 && year <= 69) {
        return 2000 + year;
    }
    if (year >= 70 && year <= 99) {
        return 1900 + year;
    }
    if (year >= 100 && year <= 199) {
        return 2000 + (year - 100);
    }
    return year;
}

[[nodiscard]] constexpr unsigned make_thread_cpuclock(int tid) noexcept {
    return kCpucLockBit | static_cast<unsigned>(tid);
}

[[nodiscard]] constexpr unsigned make_process_cpuclock(int pid) noexcept {
    return kCpucLockBit | kCpucLockProcessBit | static_cast<unsigned>(pid);
}

struct ClockIdEntry {
    int         id;
    const char* name;
};

inline constexpr ClockIdEntry kClockTable[] = {
    {kClockRealtime,      "REALTIME"},
    {kClockMonotonic,     "MONOTONIC"},
    {kClockProcessCputimeId, "CPUTIME"},
    {kClockThreadCputimeId,  "THREAD_CPUTIME"},
    {kClockMonotonicRaw,  "MONOTONIC_RAW"},
    {kClockTai,           "TAI"},
};

[[nodiscard]] inline unsigned clock_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kClockTable) / sizeof(kClockTable[0]));
}

} // namespace pbsd::kernel::timekeeping
