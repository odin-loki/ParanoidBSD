module;
#include <cstddef>

export module pbsd.userland.util.ftime;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/ftime.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly ftime_query() noexcept { return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
