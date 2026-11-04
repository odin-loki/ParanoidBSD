module;
#include <cstdint>

export module pbsd.kernel.racct;

export import pbsd.core;

/// Wave 4/5 — resource accounting from sys/racct.h, kern/kern_racct.c.
export namespace pbsd::kernel::racct {

inline constexpr int kRacctNcpu     = 0;
inline constexpr int kRacctNproc    = 1;
inline constexpr int kRacctNswap    = 2;
inline constexpr int kRacctNswapper = 3;
inline constexpr int kRacctNvmem    = 4;
inline constexpr int kRacctNfile    = 5;
inline constexpr int kRacctNsock    = 6;
inline constexpr int kRacctNpipe    = 7;
inline constexpr int kRacctNkq      = 8;
inline constexpr int kRacctNpts     = 9;
inline constexpr int kRacctNmsg     = 10;
inline constexpr int kRacctNsem     = 11;
inline constexpr int kRacctNshm     = 12;
inline constexpr int kRacctNtypes   = 13;

inline constexpr unsigned kRacctFlagInherit = 0x0001;
inline constexpr unsigned kRacctFlagOrphan  = 0x0002;

struct RacctEntry {
    int         type{};
    const char* name{};
};

inline constexpr RacctEntry kTypeTable[] = {
    {kRacctNcpu,     "cpu"},
    {kRacctNproc,    "proc"},
    {kRacctNswap,    "swap"},
    {kRacctNvmem,    "vmem"},
    {kRacctNfile,    "file"},
    {kRacctNsock,    "sock"},
    {kRacctNpipe,    "pipe"},
    {kRacctNkq,      "kqueue"},
    {kRacctNpts,     "pts"},
    {kRacctNmsg,     "msgq"},
    {kRacctNsem,     "sem"},
    {kRacctNshm,     "shm"},
};

[[nodiscard]] inline unsigned type_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kTypeTable) / sizeof(kTypeTable[0]));
}

[[nodiscard]] constexpr Status validate_type(int t) noexcept {
    if (t < 0 || t >= kRacctNtypes) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool flag_inherit(unsigned f) noexcept {
    return (f & kRacctFlagInherit) != 0;
}

} // namespace pbsd::kernel::racct
