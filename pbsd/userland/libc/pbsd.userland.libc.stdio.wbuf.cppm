module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.wbuf;

export import pbsd.core;

/// wbuf from hbsd/src/lib/libc/stdio/wbuf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wbuf_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
