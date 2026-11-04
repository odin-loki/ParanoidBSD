module;
#include <cstddef>

export module pbsd.userland.util.pw_util;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/pw_util.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly pw_util_lookup(const char* name) noexcept { if (name == nullptr) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
