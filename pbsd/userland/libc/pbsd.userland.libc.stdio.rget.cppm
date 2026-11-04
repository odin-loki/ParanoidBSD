module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.rget;

export import pbsd.core;

/// rget from hbsd/src/lib/libc/stdio/rget.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status rget_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
