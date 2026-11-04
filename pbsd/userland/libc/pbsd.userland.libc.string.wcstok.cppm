module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcstok;

export import pbsd.core;

/// wcstok from hbsd/src/lib/libc/string/wcstok.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcstok_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
