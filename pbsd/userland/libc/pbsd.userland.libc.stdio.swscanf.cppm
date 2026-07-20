module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdio.swscanf;

export import pbsd.core;

/// swscanf from hbsd/src/lib/libc/stdio/swscanf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status swscanf_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
