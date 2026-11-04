module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.vadvise;

export import pbsd.core;

/// vadvise from hbsd/src/lib/libc/sys/vadvise.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vadvise_addr(void* addr) noexcept { (void)addr; return Status::Ok; }

} // namespace pbsd::userland::libc
