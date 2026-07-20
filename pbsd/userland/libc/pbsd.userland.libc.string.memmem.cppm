module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.memmem;

export import pbsd.core;

/// memmem from hbsd/src/lib/libc/string/memmem.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* memmem_find(const void* hay, std::size_t hlen, const void* needle, std::size_t nlen) noexcept {
    (void)hay; (void)hlen; (void)needle; (void)nlen;
    return nullptr;
}

} // namespace pbsd::userland::libc
