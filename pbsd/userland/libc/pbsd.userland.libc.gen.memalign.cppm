module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.memalign;

export import pbsd.core;

/// memalign from hbsd/src/lib/libc/gen/memalign.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline void* memalign_boundary(std::size_t align, std::size_t size) noexcept { (void)align; (void)size; return nullptr; }

} // namespace pbsd::userland::libc
