module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.exect;

export import pbsd.core;

/// exect from hbsd/src/lib/libc/gen/exect.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status exect_validate(const char* path, char* const argv[]) noexcept {
    if (path == nullptr || path[0] == '\0' || argv == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
