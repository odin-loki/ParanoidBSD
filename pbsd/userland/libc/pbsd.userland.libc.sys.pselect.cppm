module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.pselect;

export import pbsd.core;

/// pselect from hbsd/src/lib/libc/sys/pselect.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status pselect_nfds(int nfds) noexcept { (void)nfds; return Status::Ok; }

} // namespace pbsd::userland::libc
