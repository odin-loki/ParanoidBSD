module;
#include <cstddef>

export module pbsd.userland.libc.gen.once_stub;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/gen/_once_stub.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly once_stub_init() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libc
