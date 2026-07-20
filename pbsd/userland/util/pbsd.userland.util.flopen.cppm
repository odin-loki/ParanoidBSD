export module pbsd.userland.util.flopen;

import pbsd.core;

/// flopen(3) race-avoidance loop from hbsd/src/lib/libutil/flopen.c
export namespace pbsd::userland::util {

struct FileStat {
    unsigned long long dev{0};
    unsigned long long ino{0};
};

using OpenAtFn = int (*)(int dirfd, const char* path, int flags, unsigned mode);
using FlockFn = int (*)(int fd, int operation);
using FstatAtFn = int (*)(int dirfd, const char* path, FileStat* out);
using FstatFn = int (*)(int fd, FileStat* out);
using FtruncateFn = int (*)(int fd, long long length);
using CloseFn = void (*)(int fd);

inline constexpr int kLockEx = 2;
inline constexpr int kLockNb = 4;
inline constexpr int kOCreat = 0x0200;
inline constexpr int kOTrunc = 0x0400;
inline constexpr int kONonblock = 0x0800;

[[nodiscard]] inline int vflopenat(int dirfd, const char* path, int flags,
                                   unsigned mode, OpenAtFn openat,
                                   FlockFn flock, FstatAtFn fstatat,
                                   FstatFn fstat, FtruncateFn ftruncate,
                                   CloseFn close_fn) noexcept {
    int operation = kLockEx;
    if ((flags & kONonblock) != 0) {
        operation |= kLockNb;
    }

    const int trunc = (flags & kOTrunc);
    flags &= ~kOTrunc;

    for (;;) {
        const int fd = openat(dirfd, path, flags, mode);
        if (fd < 0) {
            return -1;
        }
        if (flock(fd, operation) != 0) {
            close_fn(fd);
            return -1;
        }

        FileStat sb{};
        FileStat fsb{};
        if (fstatat(dirfd, path, &sb) != 0) {
            close_fn(fd);
            continue;
        }
        if (fstat(fd, &fsb) != 0) {
            close_fn(fd);
            return -1;
        }
        if (sb.dev != fsb.dev || sb.ino != fsb.ino) {
            close_fn(fd);
            continue;
        }
        if (trunc != 0 && ftruncate(fd, 0) != 0) {
            close_fn(fd);
            return -1;
        }
        return fd;
    }
}

} // namespace pbsd::userland::util
