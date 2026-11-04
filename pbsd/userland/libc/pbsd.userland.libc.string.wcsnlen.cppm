module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcsnlen;

export import pbsd.core;

/// wcsnlen from hbsd/src/lib/libc/string/wcsnlen.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcsnlen_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
