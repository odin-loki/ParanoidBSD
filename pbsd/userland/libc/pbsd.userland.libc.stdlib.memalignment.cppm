module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.memalignment;

export import pbsd.core;

/// memalignment from hbsd/src/lib/libc/stdlib/memalignment.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline std::size_t memalignment_align(std::size_t align) noexcept { return align == 0 ? 1 : align; }

} // namespace pbsd::userland::libc
