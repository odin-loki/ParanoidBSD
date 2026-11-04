module;

#include <cerrno>

export module pbsd.userland.libc.stdio.perror;

/// perror message helper from hbsd/src/lib/libc/stdio/perror.c
export namespace pbsd::userland::libc::stdio {

[[nodiscard]] inline const char* errno_message(int err) noexcept {
    switch (err) {
    case 0: return "Success";
    case EINVAL: return "Invalid argument";
    case ERANGE: return "Result too large";
    case ENOENT: return "No such file or directory";
    default: return "Unknown error";
    }
}

[[nodiscard]] inline const char* current_errno_message() noexcept {
    return errno_message(errno);
}

} // namespace pbsd::userland::libc::stdio
