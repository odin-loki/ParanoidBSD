module;
#include <cstddef>

export module pbsd.userland.libprocstat.msdosfs;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libprocstat/msdosfs.c
export namespace pbsd::userland::libprocstat {

[[nodiscard]] inline StatusOnly procstat_msdosfs() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libprocstat
