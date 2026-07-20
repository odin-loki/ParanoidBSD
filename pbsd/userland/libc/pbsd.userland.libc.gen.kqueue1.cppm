module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.kqueue1;

export import pbsd.core;

/// kqueue1 from hbsd/src/lib/libc/gen/kqueue1.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status kqueue1_flags(int flags) noexcept { (void)flags; return Status::Ok; }

} // namespace pbsd::userland::libc
