module;

#include <cctype>

export module pbsd.userland.libc.ctype.ext;

/// Extended ctype from hbsd/src/lib/libc/locale/ctype.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline int iscntrl(int c) noexcept { return std::iscntrl(c); }
[[nodiscard]] inline int ispunct(int c) noexcept { return std::ispunct(c); }
[[nodiscard]] inline int isprint(int c) noexcept { return std::isprint(c); }
[[nodiscard]] inline int isgraph(int c) noexcept { return std::isgraph(c); }
[[nodiscard]] inline int isblank(int c) noexcept { return std::isblank(c); }

} // namespace pbsd::userland::libc
