module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.select;

export import pbsd.core;

/// select from hbsd/src/lib/libc/sys/select.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status select_nfds(int nfds) noexcept { return nfds >= 0 ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
