module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.fmaximuml;

/// fmaximuml from hbsd/src/lib/msun/src/s_fmaximuml.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double fmaximuml(long double x, long double y) noexcept { return x > y ? x : y; }

} // namespace pbsd::userland::msun
