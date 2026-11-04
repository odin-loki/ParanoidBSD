module;
#include <cmath>

export module pbsd.userland.msun.llround;

/// llround from hbsd/src/lib/msun/src/s_llround.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long long llround(double x) noexcept {
    return std::llround(x);
}

} // namespace pbsd::userland::msun
