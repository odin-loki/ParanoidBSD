module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.msync;

export import pbsd.core;

/// msync from hbsd/src/lib/libc/sys/msync.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status msync_addr(void* addr, std::size_t len) noexcept { (void)addr; (void)len; return Status::Ok; }

} // namespace pbsd::userland::libc
