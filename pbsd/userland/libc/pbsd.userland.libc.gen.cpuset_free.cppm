module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.cpuset_free;

export import pbsd.core;

/// cpuset_free from hbsd/src/lib/libc/gen/cpuset_free.c
export namespace pbsd::userland::libc {

inline void cpuset_free_ptr(void* set) noexcept { (void)set; }

} // namespace pbsd::userland::libc
