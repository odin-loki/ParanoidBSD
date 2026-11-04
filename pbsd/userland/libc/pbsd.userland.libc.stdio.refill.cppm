module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.refill;

export import pbsd.core;

/// refill from hbsd/src/lib/libc/stdio/refill.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status refill_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
