module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.sync;

export import pbsd.core;

/// sync from hbsd/src/lib/libc/gen/sync.c
export namespace pbsd::userland::libc {

inline void sync_all() noexcept {}

} // namespace pbsd::userland::libc
