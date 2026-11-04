module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.csu.libc_start1;

export import pbsd.core;

/// libc_start1 from hbsd/src/lib/libc/csu/libc_start1.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status libc_start1_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
