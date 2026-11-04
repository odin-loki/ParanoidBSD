module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.cxa_thread_atexit_impl;

export import pbsd.core;

/// cxa_thread_atexit_impl from hbsd/src/lib/libc/stdlib/cxa_thread_atexit_impl.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status cxa_thread_atexit_impl_validate(void (*fn)(void*)) noexcept { return fn == nullptr ? Status::Invalid : Status::Ok; }

} // namespace pbsd::userland::libc
