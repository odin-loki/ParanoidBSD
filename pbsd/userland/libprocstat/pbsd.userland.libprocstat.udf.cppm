module;
#include <cstddef>

export module pbsd.userland.libprocstat.udf;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libprocstat/udf.c
export namespace pbsd::userland::libprocstat {

[[nodiscard]] inline StatusOnly procstat_udf() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libprocstat
