module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.sys.nanosleep;

export import pbsd.core;

/// nanosleep from hbsd/src/lib/libc/sys/nanosleep.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status nanosleep_req(const void* req) noexcept { return req ? Status::Ok : Status::Invalid; }

} // namespace pbsd::userland::libc
