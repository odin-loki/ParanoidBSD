module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcschr;

export import pbsd.core;

/// wcschr from hbsd/src/lib/libc/string/wcschr.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcschr_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
