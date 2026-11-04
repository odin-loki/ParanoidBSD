module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.compat_43.gethostid;

export import pbsd.core;

/// gethostid from hbsd/src/lib/libc/compat-43/gethostid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status gethostid_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
