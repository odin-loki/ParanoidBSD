module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.exec;

export import pbsd.core;

/// exec from hbsd/src/lib/libc/gen/exec.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status exec_validate_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
