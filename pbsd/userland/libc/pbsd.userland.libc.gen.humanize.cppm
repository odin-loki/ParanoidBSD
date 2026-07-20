module;

#include <cstddef>

export module pbsd.userland.libc.gen.humanize;

export import pbsd.core;

/// humanize_number from hbsd/src/lib/libc/gen/humanize_number.c
export namespace pbsd::userland::libc {

inline constexpr char kSuffixes[] = "KMGTPE";

[[nodiscard]] inline Result<long long> scale_value(long long value, int scale,
                                                   char& suffix_out) noexcept {
    long long v = value;
    int idx = 0;
    while (scale > 0 && v >= 1024 && idx < 6) {
        v /= 1024;
        --scale;
        ++idx;
    }
    suffix_out = idx == 0 ? '\0' : kSuffixes[idx - 1];
    return result_ok(v);
}

} // namespace pbsd::userland::libc
