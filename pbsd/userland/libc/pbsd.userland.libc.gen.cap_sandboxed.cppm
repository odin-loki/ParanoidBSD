module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.cap_sandboxed;

export import pbsd.core;

/// cap_sandboxed from hbsd/src/lib/libc/gen/cap_sandboxed.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline bool cap_sandboxed_active() noexcept { return false; }

} // namespace pbsd::userland::libc
