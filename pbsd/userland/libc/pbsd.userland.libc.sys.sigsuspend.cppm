module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.sigsuspend;

export import pbsd.core;

/// sigsuspend from hbsd/src/lib/libc/sys/sigsuspend.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status sigsuspend_mask() noexcept { return Status::NotImplemented; }

} // namespace pbsd::userland::libc
