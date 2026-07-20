module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.mtctxres;

export import pbsd.core;

/// mtctxres from hbsd/src/lib/libc/resolv/mtctxres.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status mtctxres_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
