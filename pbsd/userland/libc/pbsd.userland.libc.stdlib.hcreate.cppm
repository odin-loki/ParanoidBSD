module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.hcreate;

export import pbsd.core;

/// hcreate from hbsd/src/lib/libc/stdlib/hcreate.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status hcreate_size(std::size_t nel) noexcept { (void)nel; return Status::Ok; }

} // namespace pbsd::userland::libc
