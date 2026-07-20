module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.ppoll;

export import pbsd.core;

/// ppoll from hbsd/src/lib/libc/sys/ppoll.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ppoll_fds(void* fds, unsigned nfds) noexcept { if (!fds && nfds) return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::libc
