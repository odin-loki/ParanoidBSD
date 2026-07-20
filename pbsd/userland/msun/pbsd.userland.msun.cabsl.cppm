module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cabsl;

/// cabsl from hbsd/src/lib/msun/src/w_cabsl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long double cabsl_val(long double re, long double im) noexcept { return re; }

} // namespace pbsd::userland::msun
