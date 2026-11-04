module;
#include <cstdint>

export module pbsd.kernel.posix_shm;

export import pbsd.core;

/// Wave 4/5 — POSIX shm from sys/mman.h, kern/kern_posix_shm.c.
export namespace pbsd::kernel::posix_shm {

inline constexpr int kOCreat  = 0x0200;
inline constexpr int kOExcl   = 0x0800;
inline constexpr int kORdonly = 0x0000;
inline constexpr int kOWronly = 0x0001;
inline constexpr int kORdwr   = 0x0002;
inline constexpr int kOTrunc  = 0x0400;

inline constexpr int kProtNone = 0x00;
inline constexpr int kProtRead = 0x01;
inline constexpr int kProtWrite = 0x02;
inline constexpr int kProtExec = 0x04;

inline constexpr int kMapShared  = 0x0010;
inline constexpr int kMapPrivate = 0x0002;
inline constexpr int kMapFixed   = 0x0010;
inline constexpr int kMapAnon    = 0x1000;

inline constexpr unsigned kShmNameMax = 1024;

struct ShmOpenArgs {
    const char* name{};
    int         oflag{};
    unsigned    mode{};
    std::uint64_t size{};
};

[[nodiscard]] constexpr Status validate_shm_name_len(unsigned len) noexcept {
    if (len == 0 || len >= kShmNameMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr Status validate_prot(int prot) noexcept {
    if (prot < 0 || prot > (kProtRead | kProtWrite | kProtExec)) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr bool oflag_creat(int oflag) noexcept {
    return (oflag & kOCreat) != 0;
}

[[nodiscard]] constexpr bool oflag_excl(int oflag) noexcept {
    return (oflag & kOExcl) != 0;
}

} // namespace pbsd::kernel::posix_shm
