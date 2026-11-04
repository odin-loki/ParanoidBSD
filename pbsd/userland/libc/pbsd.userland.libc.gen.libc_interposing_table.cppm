module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.libc_interposing_table;

export import pbsd.core;

/// libc_interposing_table from hbsd/src/lib/libc/gen/libc_interposing_table.c
export namespace pbsd::userland::libc {

inline void libc_interposing_table_init() noexcept {}

} // namespace pbsd::userland::libc
