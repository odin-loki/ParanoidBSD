module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.setcontext;

export import pbsd.core;

/// setcontext from hbsd/src/lib/libc/sys/setcontext.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status setcontext_ctx(void* ctx) noexcept { (void)ctx; return Status::NotImplemented; }

} // namespace pbsd::userland::libc
