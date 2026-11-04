module;
#include <cstddef>

export module pbsd.userland.libc.stdio.vasprintf;

export import pbsd.core;

/// scaffold from hbsd/src/lib/libc/stdio/vasprintf.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline StatusOnly vasprintf_fmt(char** ret, const char* fmt) noexcept { if (ret == nullptr || fmt == nullptr) return status_err(Status::Invalid); *ret = nullptr; return status_err(Status::NotImplemented); }

} // namespace pbsd::userland::libc
