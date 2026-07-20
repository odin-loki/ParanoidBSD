module;

#include <cmath>

export module pbsd.userland.msun.trunc;

/// trunc/truncf from hbsd/src/lib/msun/src/s_trunc.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline double trunc(double x) noexcept { return std::trunc(x); }

} // namespace pbsd::userland::msun
