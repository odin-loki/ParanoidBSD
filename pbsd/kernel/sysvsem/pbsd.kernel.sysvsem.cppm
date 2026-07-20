module;
#include <cstdint>

export module pbsd.kernel.sysvsem;

export import pbsd.core;

/// Wave 4/5 — SysV semaphores from sys/sem.h, kern/kern_sem.c.
export namespace pbsd::kernel::sysvsem {

inline constexpr int kSemUndo = 0x1000;
inline constexpr int kSemR    = 0400;
inline constexpr int kSemA    = 0200;

inline constexpr int kIpcCreat  = 00001000;
inline constexpr int kIpcExcl   = 00002000;
inline constexpr int kIpcNowait = 00004000;
inline constexpr int kIpcRm     = 0;
inline constexpr int kIpcSet    = 1;
inline constexpr int kIpcStat   = 2;
inline constexpr int kIpcInfo   = 3;

inline constexpr int kSemgetall = 20;
inline constexpr int kSemgetncnt = 18;
inline constexpr int kSemgetpid  = 19;
inline constexpr int kSemgetval  = 12;
inline constexpr int kSemgetzcnt = 17;

inline constexpr unsigned kSemMax = 500;

struct SemidDs {
    std::uint16_t sem_perm_mode{};
    int           sem_perm_key{};
    unsigned      sem_nsems{};
    std::uint32_t sem_otime{};
    std::uint32_t sem_ctime{};
};

struct Sembuf {
    unsigned short sem_num{};
    short          sem_op{};
    short          sem_flg{};
};

[[nodiscard]] constexpr Status validate_sem_num(unsigned num, unsigned nsems) noexcept {
    if (num >= nsems || nsems > kSemMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_sem_op(short op) noexcept {
    if (op == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool sem_undo(unsigned flg) noexcept {
    return (flg & kSemUndo) != 0;
}

} // namespace pbsd::kernel::sysvsem
