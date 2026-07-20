module;
#include <cstddef>

export module pbsd.userland.libprocstat.zfs;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libprocstat/zfs.c
export namespace pbsd::userland::libprocstat {

[[nodiscard]] inline StatusOnly procstat_zfs() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libprocstat
