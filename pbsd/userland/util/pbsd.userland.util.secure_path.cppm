module;
#include <cstddef>

export module pbsd.userland.util.secure_path;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/_secure_path.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly secure_path_check(const char* path) noexcept { if (path == nullptr) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
