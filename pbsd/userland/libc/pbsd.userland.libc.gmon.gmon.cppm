module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gmon.gmon;

export import pbsd.core;

/// gmon from hbsd/src/lib/libc/gmon/gmon.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status gmon_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
