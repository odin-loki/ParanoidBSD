module;

#include <cerrno>
#include <cstdio>
#include <cstring>

export module pbsd.userland.util.uucplock;

/// uu_lockerr(3) + lock codes from hbsd/src/lib/libutil/uucplock.c
export namespace pbsd::userland::util {

inline constexpr int UU_LOCK_INUSE     = 1;
inline constexpr int UU_LOCK_OK        = 0;
inline constexpr int UU_LOCK_OPEN_ERR  = -1;
inline constexpr int UU_LOCK_READ_ERR  = -2;
inline constexpr int UU_LOCK_CREAT_ERR = -3;
inline constexpr int UU_LOCK_WRITE_ERR = -4;
inline constexpr int UU_LOCK_LINK_ERR  = -5;
inline constexpr int UU_LOCK_TRY_ERR   = -6;
inline constexpr int UU_LOCK_OWNER_ERR = -7;

[[nodiscard]] inline const char* uu_lockerr(int uu_lockresult) noexcept {
    static char errbuf[128];
    const char* fmt;

    switch (uu_lockresult) {
    case UU_LOCK_INUSE:
        return "device in use";
    case UU_LOCK_OK:
        return "";
    case UU_LOCK_OPEN_ERR:
        fmt = "open error: %s";
        break;
    case UU_LOCK_READ_ERR:
        fmt = "read error: %s";
        break;
    case UU_LOCK_CREAT_ERR:
        fmt = "creat error: %s";
        break;
    case UU_LOCK_WRITE_ERR:
        fmt = "write error: %s";
        break;
    case UU_LOCK_LINK_ERR:
        fmt = "link error: %s";
        break;
    case UU_LOCK_TRY_ERR:
        fmt = "too many tries: %s";
        break;
    case UU_LOCK_OWNER_ERR:
        fmt = "not locking process: %s";
        break;
    default:
        fmt = "undefined error: %s";
        break;
    }

    std::snprintf(errbuf, sizeof(errbuf), fmt, std::strerror(errno));
    return errbuf;
}

} // namespace pbsd::userland::util
