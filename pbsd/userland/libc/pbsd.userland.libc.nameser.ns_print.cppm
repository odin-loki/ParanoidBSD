module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.nameser.ns_print;

export import pbsd.core;

/// ns_print from hbsd/src/lib/libc/nameser/ns_print.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ns_print_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
