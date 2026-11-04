module;
#include <cstddef>

export module pbsd.userland.lockf;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/lockf/lockf.c — advisory lock helpers (logic-only).
export namespace pbsd::userland::usr_bin::lockf {

inline constexpr const char* kFdLockPrefix = "/dev/fd/";

[[nodiscard]] inline bool is_fd_lock_path(const char* name) noexcept {
    if (name == nullptr) {
        return false;
    }
    const char* p = kFdLockPrefix;
    while (*p != '\0') {
        if (*name++ != *p++) {
            return false;
        }
    }
    return *name != '\0';
}

[[nodiscard]] inline Result<long> parse_fd_from_path(const char* name) noexcept {
    if (!is_fd_lock_path(name)) {
        return result_err<long>(Status::Invalid);
    }
    const char* digits = name + hosted::cstrlen(kFdLockPrefix);
    if (*digits == '\0') {
        return result_err<long>(Status::Invalid);
    }
    long fd = 0;
    for (const char* p = digits; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<long>(Status::Invalid);
        }
        fd = fd * 10 + (*p - '0');
    }
    return result_ok(fd);
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv,
                                            int& optind_out) noexcept {
    if (argv == nullptr || argc < 2) {
        return result_err<int>(Status::Invalid);
    }
    optind_out = 1;
    return result_ok(0);
}

} // namespace pbsd::userland::usr_bin::lockf
