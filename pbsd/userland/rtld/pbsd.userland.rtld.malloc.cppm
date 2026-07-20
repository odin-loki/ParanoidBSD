module;

#include <cstddef>
#include <cstdlib>

export module pbsd.userland.rtld.malloc;

/// rtld_malloc concept from hbsd/src/libexec/rtld-elf/rtld_malloc.c
export namespace pbsd::userland::rtld {

[[nodiscard]] inline void* rtld_malloc(std::size_t size) noexcept {
    return std::malloc(size);
}

inline void rtld_free(void* ptr) noexcept { std::free(ptr); }

} // namespace pbsd::userland::rtld
