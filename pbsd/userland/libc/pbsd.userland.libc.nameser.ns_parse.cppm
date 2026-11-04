module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.nameser.ns_parse;

export import pbsd.core;

/// ns_parse from hbsd/src/lib/libc/nameser/ns_parse.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ns_parse_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
