module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.compat_43.killpg;

export import pbsd.core;

/// killpg from hbsd/src/lib/libc/compat-43/killpg.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status killpg_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
