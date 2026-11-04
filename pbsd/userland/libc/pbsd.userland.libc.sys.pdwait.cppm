module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.pdwait;

export import pbsd.core;

/// pdwait from hbsd/src/lib/libc/sys/pdwait.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pdwait_pid(int pid) noexcept { (void)pid; return Status::Ok; }

} // namespace pbsd::userland::libc
