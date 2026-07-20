module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.isc.ev_streams;

export import pbsd.core;

/// ev_streams from hbsd/src/lib/libc/isc/ev_streams.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status ev_streams_ok() noexcept { return Status::Ok; }

} // namespace pbsd::userland::libc
