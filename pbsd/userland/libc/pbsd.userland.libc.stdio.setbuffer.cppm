module;
#include <cstddef>

export module pbsd.userland.libc.stdio.setbuffer;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/stdio/setbuffer.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly setbuffer_config(char* buf, int size) noexcept { (void)buf; (void)size; return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libc
