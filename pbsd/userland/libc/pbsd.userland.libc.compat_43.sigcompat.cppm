module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.compat_43.sigcompat;

export import pbsd.core;

/// sigcompat from hbsd/src/lib/libc/compat-43/sigcompat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sigcompat_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
