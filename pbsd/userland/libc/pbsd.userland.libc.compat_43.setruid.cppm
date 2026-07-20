module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.compat_43.setruid;

export import pbsd.core;

/// setruid from hbsd/src/lib/libc/compat-43/setruid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status setruid_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
