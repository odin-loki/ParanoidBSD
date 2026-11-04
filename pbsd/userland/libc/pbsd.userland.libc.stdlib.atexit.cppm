module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.atexit;

export import pbsd.core;

/// atexit from hbsd/src/lib/libc/stdlib/atexit.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status atexit_validate(void (*fn)()) noexcept { return fn == nullptr ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
