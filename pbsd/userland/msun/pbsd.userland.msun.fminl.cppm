module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fminl;

/// fminl from hbsd/src/lib/msun/src/s_fminl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fminl(long double x, long double y) noexcept { return x < y ? x : y; }

} // namespace pbsd::userland::msun
