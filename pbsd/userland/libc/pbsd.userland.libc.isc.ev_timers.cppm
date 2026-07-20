module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.isc.ev_timers;

export import pbsd.core;

/// ev_timers from hbsd/src/lib/libc/isc/ev_timers.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ev_timers_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
