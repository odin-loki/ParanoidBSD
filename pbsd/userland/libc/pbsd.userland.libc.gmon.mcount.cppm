module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gmon.mcount;

export import pbsd.core;

/// mcount from hbsd/src/lib/libc/gmon/mcount.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status mcount_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
