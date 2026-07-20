module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.gen.setproctitle;

export import pbsd.core;

/// setproctitle from hbsd/src/lib/libc/gen/setproctitle.c
export namespace pbsd::userland::libc {

inline void setproctitle_fmt(const char* fmt) noexcept { (void)fmt; }

} // namespace pbsd::userland::libc
