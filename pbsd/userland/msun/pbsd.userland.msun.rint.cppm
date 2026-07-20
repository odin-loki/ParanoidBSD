module;
#include <cmath>

export module pbsd.userland.msun.rint;

/// rint from hbsd/src/lib/msun/src/s_rint.c (float lives in rintf module).
export namespace pbsd::userland::msun {

[[nodiscard]] inline double rint(double x) noexcept {
    return std::rint(x);
}

} // namespace pbsd::userland::msun
