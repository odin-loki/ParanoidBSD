module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.stdlib.hcreate_r;

export import pbsd.core;

/// hcreate_r from hbsd/src/lib/libc/stdlib/hcreate_r.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status hcreate_r_size(std::size_t nel) noexcept { (void)nel; return Status::Ok; }

} // namespace pbsd::userland::libc
