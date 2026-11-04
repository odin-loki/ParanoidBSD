module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.sigprocmask;

export import pbsd.core;

/// sigprocmask from hbsd/src/lib/libc/sys/sigprocmask.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sigprocmask_how(int how) noexcept { (void)how; return Status::Ok; }

} // namespace pbsd::userland::libc
