module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.nameser.ns_name;

export import pbsd.core;

/// ns_name from hbsd/src/lib/libc/nameser/ns_name.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ns_name_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
