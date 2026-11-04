module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.aio_suspend;

export import pbsd.core;

/// aio_suspend from hbsd/src/lib/libc/sys/aio_suspend.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status aio_suspend_wait() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
