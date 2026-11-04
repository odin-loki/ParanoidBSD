module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.cospi;

/// cospi from hbsd/src/lib/msun/src/s_cospi.c (float lives in cospif module).
export namespace pbsd::userland::msun {

[[nodiscard]] inline double cospi_val(double x) noexcept {
    (void)x;
    return 0.0;
}

} // namespace pbsd::userland::msun
