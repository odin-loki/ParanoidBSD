module;

#include <cstddef>

export module pbsd.userland.libc.gen.getprogname;

/// getprogname from hbsd/src/lib/libc/gen/getprogname.c
export namespace pbsd::userland::libc {

inline char g_progname_buf[64] = "pbsd";

[[nodiscard]] inline const char* getprogname() noexcept { return g_progname_buf; }

} // namespace pbsd::userland::libc
