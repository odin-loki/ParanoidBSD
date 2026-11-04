module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.compat_43.getwd;

export import pbsd.core;

/// getwd from hbsd/src/lib/libc/compat-43/getwd.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getwd_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
