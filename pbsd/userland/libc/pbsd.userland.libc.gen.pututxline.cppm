module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.pututxline;

export import pbsd.core;

/// pututxline from hbsd/src/lib/libc/gen/pututxline.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pututxline_entry(const void* utx) noexcept { return utx ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
