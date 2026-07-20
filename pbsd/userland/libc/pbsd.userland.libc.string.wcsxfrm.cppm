module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.string.wcsxfrm;

export import pbsd.core;

/// wcsxfrm from hbsd/src/lib/libc/string/wcsxfrm.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status wcsxfrm_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
