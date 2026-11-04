module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cabs;

/// cabs from hbsd/src/lib/msun/src/w_cabs.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cabs_val(double re, double im) noexcept { return re; }

} // namespace pbsd::userland::msun
