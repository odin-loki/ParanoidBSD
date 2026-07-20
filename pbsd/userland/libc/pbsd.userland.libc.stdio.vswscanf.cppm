module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.vswscanf;

export import pbsd.core;

/// vswscanf from hbsd/src/lib/libc/stdio/vswscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status vswscanf_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
