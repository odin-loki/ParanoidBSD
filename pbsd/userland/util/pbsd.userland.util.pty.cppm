module;
#include <cstddef>

export module pbsd.userland.util.pty;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libutil/pty.c
export namespace pbsd::userland::util {

[[nodiscard]] inline StatusOnly pty_open(int* amaster, int* aslave) noexcept { if (amaster == nullptr || aslave == nullptr) return status_err(Status::Invalid); return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::util
