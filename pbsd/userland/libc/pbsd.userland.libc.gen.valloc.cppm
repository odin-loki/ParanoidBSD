module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.valloc;

export import pbsd.core;

/// valloc from hbsd/src/lib/libc/gen/valloc.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* valloc_size(std::size_t size) noexcept { (void)size; return nullptr; }

} // namespace pbsd::userland::libc
