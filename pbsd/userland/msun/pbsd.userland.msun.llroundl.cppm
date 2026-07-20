module;
#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.llroundl;

/// llroundl from hbsd/src/lib/msun/src/s_llroundl.c
export namespace pbsd::userland::msun {

[[nodiscard]] inline long long llroundl_val(long double x) noexcept { return static_cast<long long>(x); }

} // namespace pbsd::userland::msun
