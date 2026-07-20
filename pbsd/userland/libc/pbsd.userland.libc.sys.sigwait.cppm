module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.sigwait;

export import pbsd.core;

/// sigwait from hbsd/src/lib/libc/sys/sigwait.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sigwait_set() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
