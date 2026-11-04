module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.disklabel;

export import pbsd.core;

/// disklabel from hbsd/src/lib/libc/gen/disklabel.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status disklabel_dev(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
