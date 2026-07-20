module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.stack_protector_compat;

export import pbsd.core;

/// stack_protector_compat from hbsd/src/lib/libc/secure/stack_protector_compat.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status stack_protector_compat_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
