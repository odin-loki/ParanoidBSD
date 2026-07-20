module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.swapcontext;

export import pbsd.core;

/// swapcontext from hbsd/src/lib/libc/sys/swapcontext.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status swapcontext_ctx(void* oucp, void* ucp) noexcept { (void)oucp; (void)ucp; return Status::NotImplemented; }

} // namespace pbsd::userland::libc
