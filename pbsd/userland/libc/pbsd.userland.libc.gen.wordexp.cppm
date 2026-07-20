module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.wordexp;

export import pbsd.core;

/// wordexp from hbsd/src/lib/libc/gen/wordexp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wordexp_words(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
