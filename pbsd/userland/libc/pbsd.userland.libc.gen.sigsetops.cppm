module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.sigsetops;

export import pbsd.core;

/// sigsetops from hbsd/src/lib/libc/gen/sigsetops.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool sigset_empty() noexcept { return true; }

} // namespace pbsd::userland::libc
