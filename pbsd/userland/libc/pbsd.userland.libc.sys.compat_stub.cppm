module;
#include <cstddef>

export module pbsd.userland.libc.sys.compat_stub;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/sys/compat_stub.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status compat_stub() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
