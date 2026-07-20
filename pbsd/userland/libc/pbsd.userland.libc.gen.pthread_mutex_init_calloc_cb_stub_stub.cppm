module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.pthread_mutex_init_calloc_cb_stub_stub;

export import pbsd.core;

/// pthread_mutex_init_calloc_cb_stub_stub from hbsd/src/lib/libc/gen/__pthread_mutex_init_calloc_cb_stub.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pthread_mutex_init_calloc_cb_stub_stub_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
