module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gdtoa.machdep_ldisx;

export import pbsd.core;

/// machdep_ldisx from hbsd/src/lib/libc/gdtoa/machdep_ldisx.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status machdep_ldisx_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
