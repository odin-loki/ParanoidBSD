module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.memfd_create;

export import pbsd.core;

/// memfd_create from hbsd/src/lib/libc/gen/memfd_create.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status memfd_create_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
