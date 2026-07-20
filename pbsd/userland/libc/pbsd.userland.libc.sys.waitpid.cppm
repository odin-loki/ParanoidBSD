module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.waitpid;

export import pbsd.core;

/// waitpid from hbsd/src/lib/libc/sys/waitpid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status waitpid_pid(int pid) noexcept { (void)pid; return Status::Ok; }

} // namespace pbsd::userland::libc
