module;

#include <cstdint>
#include <cstring>

export module pbsd.userland.msun.nan;

/// nan/nanf from hbsd/src/lib/msun/src/s_nan.c (quiet NaN payload)
export namespace pbsd::userland::msun {

[[nodiscard]] inline double nan(const char* tag) noexcept {
    (void)tag;
    std::uint64_t bits = 0x7FF8000000000001ULL;
    double result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

[[nodiscard]] inline float nanf(const char* tag) noexcept {
    (void)tag;
    std::uint32_t bits = 0x7FC00001U;
    float result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

} // namespace pbsd::userland::msun
