module;
#include <cstddef>

export module pbsd.userland.util.gr_util;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/gr_util.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly gr_util_lookup(const char* name) noexcept { if (name == nullptr) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
