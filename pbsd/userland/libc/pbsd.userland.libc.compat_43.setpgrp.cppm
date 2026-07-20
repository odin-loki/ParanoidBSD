module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.compat_43.setpgrp;

export import pbsd.core;

/// setpgrp from hbsd/src/lib/libc/compat-43/setpgrp.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status setpgrp_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
