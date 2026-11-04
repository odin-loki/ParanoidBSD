module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.popen;

export import pbsd.core;

/// popen from hbsd/src/lib/libc/gen/popen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status popen_cmd(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
