module;

#include <cstdlib>

export module pbsd.userland.libc.stdlib.reallocf;

/// reallocf concept from hbsd/src/lib/libc/stdlib/reallocf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* reallocf(void* ptr, std::size_t size) noexcept {
    void* np = std::realloc(ptr, size);
    if (np == nullptr && size != 0) {
        std::free(ptr);
    }
    return np;
}

} // namespace pbsd::userland::libc
