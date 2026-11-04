module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.resolv.res_send;

export import pbsd.core;

/// res_send from hbsd/src/lib/libc/resolv/res_send.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status res_send_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
