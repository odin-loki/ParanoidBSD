module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcscoll;

export import pbsd.core;

/// wcscoll from hbsd/src/lib/libc/string/wcscoll.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcscoll_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
