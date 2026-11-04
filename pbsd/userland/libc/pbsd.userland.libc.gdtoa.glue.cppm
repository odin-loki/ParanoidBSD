module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gdtoa.glue;

export import pbsd.core;

/// glue from hbsd/src/lib/libc/gdtoa/glue.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status glue_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
