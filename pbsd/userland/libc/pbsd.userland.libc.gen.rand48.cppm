module;
#include <cstddef>

export module pbsd.userland.libc.gen.rand48;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/gen/_rand48.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly rand48_seed(unsigned short seed[3]) noexcept { (void)seed; return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libc
