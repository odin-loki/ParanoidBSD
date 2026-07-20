module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.nameser.ns_samedomain;

export import pbsd.core;

/// ns_samedomain from hbsd/src/lib/libc/nameser/ns_samedomain.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ns_samedomain_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
