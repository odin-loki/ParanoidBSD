module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gdtoa.machdep_ldisQ;

export import pbsd.core;

/// machdep_ldisQ from hbsd/src/lib/libc/gdtoa/machdep_ldisQ.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status machdep_ldisQ_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
