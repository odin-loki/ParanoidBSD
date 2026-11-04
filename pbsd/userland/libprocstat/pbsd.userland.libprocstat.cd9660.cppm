module;
#include <cstddef>

export module pbsd.userland.libprocstat.cd9660;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libprocstat/cd9660.c
export namespace pbsd::userland::libprocstat {

[[nodiscard]] inline StatusOnly procstat_cd9660() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libprocstat
