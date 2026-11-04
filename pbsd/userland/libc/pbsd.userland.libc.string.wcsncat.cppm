module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcsncat;

export import pbsd.core;

/// wcsncat from hbsd/src/lib/libc/string/wcsncat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcsncat_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
