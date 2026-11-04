module;
#include <cstddef>

export module pbsd.userland.libprocstat.compat;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libprocstat/libprocstat_compat.c
export namespace pbsd::userland::libprocstat {

[[nodiscard]] inline StatusOnly procstat_compat() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libprocstat
