module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.fpathconf;

export import pbsd.core;

/// fpathconf from hbsd/src/lib/libc/gen/fpathconf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<long> fpathconf_fd(int fd, int name) noexcept {
    if (fd < 0) {
        return result_err<long>(Status::Invalid);
    }
    (void)name;
    return result_ok(0L);
}

} // namespace pbsd::userland::libc
