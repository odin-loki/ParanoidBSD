module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.pathconf;

export import pbsd.core;

/// pathconf from hbsd/src/lib/libc/gen/pathconf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Result<long> pathconf_name(const char* path, int name) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return result_err<long>(Status::Invalid);
    }
    (void)name;
    return result_ok(0L);
}

} // namespace pbsd::userland::libc
