module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.brk;

export import pbsd.core;

/// brk from hbsd/src/lib/libc/sys/brk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status brk_addr(void* addr) noexcept { (void)addr; return Status::Ok; }

} // namespace pbsd::userland::libc
