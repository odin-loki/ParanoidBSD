module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.nameser.ns_ttl;

export import pbsd.core;

/// ns_ttl from hbsd/src/lib/libc/nameser/ns_ttl.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ns_ttl_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
