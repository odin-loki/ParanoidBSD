module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.getmntinfo_compat11;

export import pbsd.core;

/// getmntinfo_compat11 from hbsd/src/lib/libc/gen/getmntinfo_compat11.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status getmntinfo_compat11_flags(int flags) noexcept { (void)flags; return Status::Ok; }

} // namespace pbsd::userland::libc
