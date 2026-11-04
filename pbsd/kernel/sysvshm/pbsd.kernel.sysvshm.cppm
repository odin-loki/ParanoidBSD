module;
#include <cstdint>

export module pbsd.kernel.sysvshm;

export import pbsd.core;

/// Wave 4/5 — SysV shared memory from sys/shm.h, kern/kern_shm.c.
export namespace pbsd::kernel::sysvshm {

inline constexpr int kShmR = 0400;
inline constexpr int kShmW = 0200;

inline constexpr int kIpcCreat  = 00001000;
inline constexpr int kIpcExcl   = 00002000;
inline constexpr int kIpcNowait = 00004000;
inline constexpr int kIpcRm    = 00000000;
inline constexpr int kIpcSet    = 00000001;
inline constexpr int kIpcStat   = 00000002;
inline constexpr int kIpcInfo   = 00000003;

inline constexpr int kShmLock   = 11;
inline constexpr int kShmUnlock = 12;
inline constexpr int kShmRdonly = 010000;
inline constexpr int kShmRnd    = 020000;
inline constexpr int kShmAnon   = 040000;

inline constexpr int kShmmax = 0;
inline constexpr int kShmmin = 1;
inline constexpr int kShmmni = 2;
inline constexpr int kShmseg = 3;
inline constexpr int kShmall = 4;

struct ShmidDs {
    std::uint64_t shm_segsz{};
    std::uint32_t shm_cpid{};
    std::uint32_t shm_lpid{};
    std::uint16_t shm_nattch{};
    std::uint16_t shm_perm_mode{};
    int           shm_perm_key{};
};

[[nodiscard]] constexpr Status validate_shm_size(std::uint64_t size) noexcept {
    if (size == 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_ipc_cmd(int cmd) noexcept {
    const int op = cmd & 0xff;
    if (op >= 0 && op <= 3) {
        return Status::Ok;
    }
    if (op == kShmLock || op == kShmUnlock) {
        return Status::Ok;
    }
    return Status::Invalid;
}

[[nodiscard]] constexpr bool ipc_creat(int cmd) noexcept {
    return (cmd & kIpcCreat) != 0;
}

} // namespace pbsd::kernel::sysvshm
