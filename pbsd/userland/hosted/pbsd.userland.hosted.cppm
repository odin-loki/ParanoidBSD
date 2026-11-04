module;

#if defined(__linux__)
#define _GNU_SOURCE
#endif

#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#if defined(__FreeBSD__) || defined(__linux__) || defined(__APPLE__)
#define PBSD_HOSTED_POSIX 1
#include <unistd.h>
#include <sys/uio.h>
#endif

#if defined(__FreeBSD__)
#include <sys/cpuset.h>
#endif

#if defined(__linux__)
#include <sched.h>
#endif

#ifndef _WIN32
#include <stdlib.h>
#endif

export module pbsd.userland.hosted;

export import pbsd.core;

export namespace pbsd::userland::hosted {

[[nodiscard]] inline Status errno_status() noexcept {
    switch (errno) {
    case 0:
        return Status::Ok;
    case EACCES:
    case EPERM:
        return Status::Denied;
    case ENOENT:
        return Status::NotFound;
    case EBUSY:
        return Status::Busy;
    case ENOMEM:
        return Status::NoMemory;
    case ETIMEDOUT:
        return Status::Timeout;
    case EINVAL:
        return Status::Invalid;
    default:
        return Status::Protocol;
    }
}

[[nodiscard]] inline StatusOnly map_errno(int rc) noexcept {
    if (rc == 0) {
        return status_ok();
    }
    return status_err(errno_status());
}

/// Freestanding-friendly string helpers (no std dependency).
[[nodiscard]] inline std::size_t cstrlen(const char* s) noexcept {
    if (s == nullptr) {
        return 0;
    }
    std::size_t n = 0;
    while (s[n] != '\0') {
        ++n;
    }
    return n;
}

[[nodiscard]] inline int cstrcmp(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) {
        return (a == b) ? 0 : 1;
    }
    while (*a != '\0' && *a == *b) {
        ++a;
        ++b;
    }
    return static_cast<unsigned char>(*a) - static_cast<unsigned char>(*b);
}

[[nodiscard]] inline char* cstrchr(char* s, int ch) noexcept {
    if (s == nullptr) {
        return nullptr;
    }
    for (; *s != '\0'; ++s) {
        if (*s == static_cast<char>(ch)) {
            return s;
        }
    }
    return (*s == static_cast<char>(ch)) ? s : nullptr;
}

[[nodiscard]] inline const char* cstrchr(const char* s, int ch) noexcept {
    return cstrchr(const_cast<char*>(s), ch);
}

struct Iovec {
    void* base{nullptr};
    std::size_t len{0};
};

#if defined(PBSD_HOSTED_POSIX)
constexpr int kIovMax =
#if defined(IOV_MAX)
    IOV_MAX;
#else
    1024;
#endif

[[nodiscard]] inline StatusOnly writev_all(int fd, Iovec* iov, int count) noexcept {
    if (fd < 0 || iov == nullptr || count < 0) {
        return status_err(Status::Invalid);
    }
    int remaining = count;
    while (remaining > 0) {
        const int chunk = (remaining > kIovMax) ? kIovMax : remaining;
        struct iovec sys_iov[1024];
        if (chunk > static_cast<int>(sizeof(sys_iov) / sizeof(sys_iov[0]))) {
            return status_err(Status::Invalid);
        }
        for (int i = 0; i < chunk; ++i) {
            sys_iov[i].iov_base = iov[i].base;
            sys_iov[i].iov_len = iov[i].len;
        }
        const ssize_t n = ::writev(fd, sys_iov, chunk);
        if (n < 0) {
            return map_errno(-1);
        }
        iov += chunk;
        remaining -= chunk;
    }
    return status_ok();
}

[[nodiscard]] inline StatusOnly write_all(int fd, const char* data, std::size_t len) noexcept {
    if (fd < 0 || (data == nullptr && len != 0)) {
        return status_err(Status::Invalid);
    }
    std::size_t off = 0;
    while (off < len) {
        const ssize_t n = ::write(fd, data + off, len - off);
        if (n < 0) {
            return map_errno(-1);
        }
        if (n == 0) {
            return status_err(Status::Protocol);
        }
        off += static_cast<std::size_t>(n);
    }
    return status_ok();
}

[[nodiscard]] inline StatusOnly write_line(int fd, const char* s) noexcept {
    const auto st = write_all(fd, s, cstrlen(s));
    if (!ok(st.status)) {
        return st;
    }
    return write_all(fd, "\n", 1);
}

[[nodiscard]] inline int processors_conf() noexcept {
    const long n = ::sysconf(_SC_NPROCESSORS_CONF);
    return (n < 0) ? -1 : static_cast<int>(n);
}

[[nodiscard]] inline int processors_online() noexcept {
    const long n = ::sysconf(_SC_NPROCESSORS_ONLN);
    return (n < 0) ? -1 : static_cast<int>(n);
}

[[nodiscard]] inline StatusOnly get_hostname(char* buf, std::size_t len) noexcept {
    if (buf == nullptr || len == 0) {
        return status_err(Status::Invalid);
    }
#if defined(__FreeBSD__) || defined(__linux__) || defined(__APPLE__)
    if (::gethostname(buf, len) != 0) {
        return map_errno(-1);
    }
    buf[len - 1] = '\0';
    return status_ok();
#else
    (void)len;
    buf[0] = '\0';
    return status_err(Status::Protocol);
#endif
}

[[nodiscard]] inline StatusOnly set_hostname(const char* name) noexcept {
    if (name == nullptr) {
        return status_err(Status::Invalid);
    }
#if defined(__FreeBSD__) || defined(__linux__) || defined(__APPLE__)
    if (::sethostname(name, static_cast<int>(cstrlen(name))) != 0) {
        return map_errno(-1);
    }
    return status_ok();
#else
    (void)name;
    return status_err(Status::Protocol);
#endif
}

[[nodiscard]] inline StatusOnly get_domainname(char* buf, std::size_t len) noexcept {
    if (buf == nullptr || len == 0) {
        return status_err(Status::Invalid);
    }
#if defined(__FreeBSD__)
    if (::getdomainname(buf, static_cast<int>(len)) != 0) {
        return map_errno(-1);
    }
    buf[len - 1] = '\0';
    return status_ok();
#elif defined(__linux__)
    if (::getdomainname(buf, len) != 0) {
        return map_errno(-1);
    }
    buf[len - 1] = '\0';
    return status_ok();
#else
    (void)len;
    buf[0] = '\0';
    return status_err(Status::Protocol);
#endif
}

[[nodiscard]] inline StatusOnly set_domainname(const char* name) noexcept {
    if (name == nullptr) {
        return status_err(Status::Invalid);
    }
#if defined(__FreeBSD__)
    if (::setdomainname(name, static_cast<int>(cstrlen(name))) != 0) {
        return map_errno(-1);
    }
    return status_ok();
#elif defined(__linux__)
    if (::setdomainname(name, cstrlen(name)) != 0) {
        return map_errno(-1);
    }
    return status_ok();
#else
    (void)name;
    return status_err(Status::Protocol);
#endif
}

[[nodiscard]] inline StatusOnly get_cwd(char* buf, std::size_t len) noexcept {
    if (buf == nullptr || len == 0) {
        return status_err(Status::Invalid);
    }
    if (::getcwd(buf, len) == nullptr) {
        return map_errno(-1);
    }
    return status_ok();
}

[[nodiscard]] inline const char* getenv_cstr(const char* name) noexcept {
#if defined(_WIN32)
    (void)name;
    return nullptr;
#else
    return std::getenv(name);
#endif
}

[[nodiscard]] inline int affinity_cpu_count() noexcept {
#if defined(__FreeBSD__)
    cpuset_t mask;
    CPU_ZERO(&mask);
    if (cpuset_getaffinity(CPU_LEVEL_WHICH, CPU_WHICH_TID, -1, sizeof(mask), &mask) != 0) {
        return -1;
    }
    return CPU_COUNT(&mask);
#elif defined(__linux__)
    cpu_set_t set;
    CPU_ZERO(&set);
    if (sched_getaffinity(0, sizeof(set), &set) != 0) {
        return -1;
    }
    return CPU_COUNT(&set);
#else
    return processors_online();
#endif
}

#else // !PBSD_HOSTED_POSIX

constexpr int kIovMax = 1024;

[[nodiscard]] inline StatusOnly writev_all(int /*fd*/, Iovec* /*iov*/, int /*count*/) noexcept {
    return status_err(Status::Protocol);
}

[[nodiscard]] inline StatusOnly write_all(int /*fd*/, const char* /*data*/,
                                          std::size_t /*len*/) noexcept {
    return status_err(Status::Protocol);
}

[[nodiscard]] inline StatusOnly write_line(int /*fd*/, const char* /*s*/) noexcept {
    return status_err(Status::Protocol);
}

[[nodiscard]] inline int processors_conf() noexcept { return -1; }
[[nodiscard]] inline int processors_online() noexcept { return -1; }
[[nodiscard]] inline StatusOnly get_hostname(char* /*buf*/, std::size_t /*len*/) noexcept {
    return status_err(Status::Protocol);
}
[[nodiscard]] inline StatusOnly set_hostname(const char* /*name*/) noexcept {
    return status_err(Status::Protocol);
}
[[nodiscard]] inline StatusOnly get_domainname(char* /*buf*/, std::size_t /*len*/) noexcept {
    return status_err(Status::Protocol);
}
[[nodiscard]] inline StatusOnly set_domainname(const char* /*name*/) noexcept {
    return status_err(Status::Protocol);
}
[[nodiscard]] inline StatusOnly get_cwd(char* /*buf*/, std::size_t /*len*/) noexcept {
    return status_err(Status::Protocol);
}
[[nodiscard]] inline const char* getenv_cstr(const char* /*name*/) noexcept { return nullptr; }
[[nodiscard]] inline int affinity_cpu_count() noexcept { return -1; }

#endif

constexpr unsigned kMaxHostnameLen = 256;
constexpr unsigned kMaxPathLen = 4096;

} // namespace pbsd::userland::hosted
