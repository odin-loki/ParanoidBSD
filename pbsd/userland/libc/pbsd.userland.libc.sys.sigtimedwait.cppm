module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.sigtimedwait;

export import pbsd.core;

/// sigtimedwait from hbsd/src/lib/libc/sys/sigtimedwait.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sigtimedwait_set() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
