module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.compat_43.setrgid;

export import pbsd.core;

/// setrgid from hbsd/src/lib/libc/compat-43/setrgid.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status setrgid_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
