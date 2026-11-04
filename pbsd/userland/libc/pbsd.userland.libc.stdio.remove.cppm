module;
#include <cstddef>

export module pbsd.userland.libc.stdio.remove;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/stdio/remove.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly remove_path(const char* path) noexcept { if (path == nullptr) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libc
