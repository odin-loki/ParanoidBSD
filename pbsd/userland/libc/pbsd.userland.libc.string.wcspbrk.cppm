module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcspbrk;

export import pbsd.core;

/// wcspbrk from hbsd/src/lib/libc/string/wcspbrk.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcspbrk_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
