module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.nameser.ns_netint;

export import pbsd.core;

/// ns_netint from hbsd/src/lib/libc/nameser/ns_netint.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ns_netint_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
