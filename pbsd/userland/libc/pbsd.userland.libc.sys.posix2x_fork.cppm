module;
#include <cstddef>

export module pbsd.userland.libc.sys.posix2x_fork;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/sys/POSIX2x_Fork.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly posix2x_fork() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libc
