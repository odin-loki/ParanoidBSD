module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.net.ntoh;

export import pbsd.core;

/// ntoh from hbsd/src/lib/libc/net/ntoh.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ntoh_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
