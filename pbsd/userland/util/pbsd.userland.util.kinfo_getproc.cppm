module;
#include <cstddef>

export module pbsd.userland.util.kinfo_getproc;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/kinfo_getproc.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly kinfo_getproc_query(int pid) noexcept { if (pid < 0) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
