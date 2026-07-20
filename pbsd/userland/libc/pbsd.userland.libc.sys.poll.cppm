module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.poll;

export import pbsd.core;

/// poll from hbsd/src/lib/libc/sys/poll.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status poll_fds(void* fds, unsigned nfds) noexcept { if (!fds && nfds) return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::libc
