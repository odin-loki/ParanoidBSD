module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.spinlock_stub_impl;

export import pbsd.core;

/// spinlock_stub_impl from hbsd/src/lib/libc/gen/_spinlock_stub.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status spinlock_stub_impl_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
