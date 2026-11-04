module;

#include <cerrno>
#include <cstddef>
#include <cstdint>

#if defined(__FreeBSD__)
#include <sys/capsicum.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <nl_types.h>
#endif

export module pbsd.userland.capsicum.os;

export import pbsd.core;
export import pbsd.userland.hosted;

export namespace pbsd::userland::capsicum::os {

#if defined(__FreeBSD__)

struct CapRights {
    cap_rights_t raw{};

    [[nodiscard]] static CapRights init_base() noexcept {
        CapRights r{};
        cap_rights_init(&r.raw, CAP_EVENT, CAP_FCNTL, CAP_FSTAT, CAP_IOCTL, CAP_SEEK);
        return r;
    }

    void add(unsigned long long cap) noexcept { cap_rights_set(&raw, cap); }
};

constexpr unsigned long kStreamIoctlCmds[] = {
#ifdef TIOCGETA
    TIOCGETA,
#endif
#ifdef TIOCGWINSZ
    TIOCGWINSZ,
#endif
#ifdef FIODTYPE
    FIODTYPE,
#endif
};

constexpr std::size_t kStreamIoctlCount = sizeof(kStreamIoctlCmds) / sizeof(kStreamIoctlCmds[0]);
constexpr std::uint32_t kStreamFcntlRights = CAP_FCNTL_GETFL;

[[nodiscard]] inline StatusOnly rights_limit(int fd, const CapRights& rights) noexcept {
    if (fd < 0) {
        return status_err(Status::Invalid);
    }
    if (cap_rights_limit(fd, &rights.raw) < 0) {
        if (errno == ENOSYS) {
            return status_ok();
        }
        return hosted::map_errno(-1);
    }
    return status_ok();
}

[[nodiscard]] inline StatusOnly ioctls_limit(int fd) noexcept {
    if (fd < 0) {
        return status_err(Status::Invalid);
    }
    if (kStreamIoctlCount == 0) {
        return status_ok();
    }
    if (cap_ioctls_limit(fd, kStreamIoctlCmds, kStreamIoctlCount) < 0) {
        if (errno == ENOSYS) {
            return status_ok();
        }
        return hosted::map_errno(-1);
    }
    return status_ok();
}

[[nodiscard]] inline StatusOnly fcntls_limit(int fd) noexcept {
    if (fd < 0) {
        return status_err(Status::Invalid);
    }
    if (cap_fcntls_limit(fd, kStreamFcntlRights) < 0) {
        if (errno == ENOSYS) {
            return status_ok();
        }
        return hosted::map_errno(-1);
    }
    return status_ok();
}

[[nodiscard]] inline StatusOnly cap_enter() noexcept {
    if (::cap_enter() < 0) {
        if (errno == ENOSYS) {
            return status_ok();
        }
        return hosted::map_errno(-1);
    }
    return status_ok();
}

inline void cache_tzdata() noexcept {
    tzset();
    time_t delta = 0;
    (void)gmtime(&delta);
}

inline void cache_catpages() noexcept { (void)catopen("libc", NL_CAT_LOCALE); }

#else // !__FreeBSD__

struct CapRights {
    unsigned long long bits{0};
    [[nodiscard]] static CapRights init_base() noexcept { return CapRights{}; }
    void add(unsigned long long cap) noexcept { bits |= cap; }
};

[[nodiscard]] inline StatusOnly rights_limit(int /*fd*/, const CapRights& /*rights*/) noexcept {
    return status_ok();
}

[[nodiscard]] inline StatusOnly ioctls_limit(int /*fd*/) noexcept { return status_ok(); }

[[nodiscard]] inline StatusOnly fcntls_limit(int /*fd*/) noexcept { return status_ok(); }

[[nodiscard]] inline StatusOnly cap_enter() noexcept { return status_ok(); }

inline void cache_tzdata() noexcept {}
inline void cache_catpages() noexcept {}

#endif

} // namespace pbsd::userland::capsicum::os
