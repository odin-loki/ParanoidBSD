module;
#include <cstddef>

export module pbsd.userland.libprocstat.core;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libprocstat/core.c
export namespace pbsd::userland::libprocstat {

[[nodiscard]] inline StatusOnly procstat_core_open(const char* path) noexcept { if (path == nullptr) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libprocstat
