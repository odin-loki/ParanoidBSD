module;
#include <cstddef>

export module pbsd.userland.libprocstat.smbfs;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libprocstat/smbfs.c
export namespace pbsd::userland::libprocstat {

[[nodiscard]] inline StatusOnly procstat_smbfs() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libprocstat
