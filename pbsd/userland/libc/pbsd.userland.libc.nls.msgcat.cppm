module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.nls.msgcat;

export import pbsd.core;

/// msgcat from hbsd/src/lib/libc/nls/msgcat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status msgcat_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
