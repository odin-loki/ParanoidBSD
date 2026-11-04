module;
#include <cstdint>

export module pbsd.kernel.vfs_mount;

export import pbsd.core;

/// Freestanding port of `sys/mount.h` / `kern/vfs_mount.c`.
export namespace pbsd::kernel::vfs_mount {

inline constexpr unsigned kFsNameLen = 16;
inline constexpr unsigned kMountNameLen = 1024;
inline constexpr unsigned kStatFsVersion = 0x20140518;

inline constexpr std::uint64_t kRdOnly = 0x0000000000000001ULL;
inline constexpr std::uint64_t kSync = 0x0000000000000002ULL;
inline constexpr std::uint64_t kNoExec = 0x0000000000000004ULL;
inline constexpr std::uint64_t kNoSuid = 0x0000000000000008ULL;
inline constexpr std::uint64_t kAsync = 0x0000000000000040ULL;
inline constexpr std::uint64_t kLocal = 0x0000000000001000ULL;
inline constexpr std::uint64_t kNoAtime = 0x0000000010000000ULL;

struct Fsid {
    int val[2]{};
};

[[nodiscard]] inline bool fsid_cmp(const Fsid& a, const Fsid& b) noexcept {
    return a.val[0] != b.val[0] || a.val[1] != b.val[1];
}

[[nodiscard]] inline bool has_flag(std::uint64_t flags, std::uint64_t bit) noexcept {
    return (flags & bit) != 0;
}

[[nodiscard]] inline Status validate_flags(std::uint64_t flags) noexcept {
    (void)flags;
    return Status::Ok;
}

} // namespace pbsd::kernel::vfs_mount
