module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.compat_43.sethostid;

export import pbsd.core;

/// sethostid from hbsd/src/lib/libc/compat-43/sethostid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sethostid_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
