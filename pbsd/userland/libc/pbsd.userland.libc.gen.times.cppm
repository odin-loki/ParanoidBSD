module;
#include <cstdint>

export module pbsd.userland.libc.gen.times;

export import pbsd.core;

/// times from hbsd/src/lib/libc/gen/times.c
export namespace pbsd::userland::libc {

struct Tms {
    std::int64_t tms_utime{0};
    std::int64_t tms_stime{0};
    std::int64_t tms_cutime{0};
    std::int64_t tms_cstime{0};
};

[[nodiscard]] inline Tms times_zero() noexcept { return Tms{}; }

} // namespace pbsd::userland::libc
