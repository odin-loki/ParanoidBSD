module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcslcat;

export import pbsd.core;

/// wcslcat from hbsd/src/lib/libc/string/wcslcat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcslcat_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
