module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.secure.libc_stack_protector;

export import pbsd.core;

/// libc_stack_protector from hbsd/src/lib/libc/secure/libc_stack_protector.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status libc_stack_protector_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
