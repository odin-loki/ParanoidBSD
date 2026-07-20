module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.sysctl;

export import pbsd.core;

/// sysctl from hbsd/src/lib/libc/gen/sysctl.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sysctl_mib(const int* mib, unsigned len) noexcept { if (!mib || !len) return Status::Invalid; return Status::Ok; }

} // namespace pbsd::userland::libc
