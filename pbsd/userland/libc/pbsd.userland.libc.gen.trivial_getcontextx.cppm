module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.trivial_getcontextx;

export import pbsd.core;

/// trivial_getcontextx from hbsd/src/lib/libc/gen/trivial_getcontextx.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status trivial_getcontextx_ctx(void* ctx) noexcept { (void)ctx; return Status::Ok; }

} // namespace pbsd::userland::libc
