export module pbsd.userland.util.secure;

import pbsd.core;

/// _secure_path(3) from hbsd/src/lib/libutil/_secure_path.c
export namespace pbsd::userland::util {

inline constexpr int kSecurePathOk       = 0;
inline constexpr int kSecurePathFail     = -1;
inline constexpr int kSecurePathMissing  = -2;

struct PathStat {
    bool exists{false};
    bool is_regular{false};
    bool world_writable{false};
    unsigned uid{0};
    unsigned gid{0};
    bool group_writable{false};
};

using LstatFn = int (*)(const char* path, PathStat* out) noexcept;

[[nodiscard]] inline int secure_path(const char* path, int uid, int gid,
                                     LstatFn lstat) noexcept {
    if (path == nullptr || lstat == nullptr) {
        return kSecurePathFail;
    }

    PathStat sb{};
    if (lstat(path, &sb) != 0) {
        return kSecurePathMissing;
    }
    if (!sb.exists) {
        return kSecurePathMissing;
    }
    if (!sb.is_regular) {
        return kSecurePathFail;
    }
    if (sb.world_writable) {
        return kSecurePathFail;
    }
    if (uid != -1 && sb.uid != static_cast<unsigned>(uid) && sb.uid != 0) {
        return kSecurePathFail;
    }
    if (gid != -1 && sb.gid != static_cast<unsigned>(gid) && sb.group_writable) {
        return kSecurePathFail;
    }
    return kSecurePathOk;
}

} // namespace pbsd::userland::util
