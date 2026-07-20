module;

#include <cstdint>

export module pbsd.userland.libc.gen.utime;

/// utime/utimes concepts from hbsd/src/lib/libc/gen/utime.c
export namespace pbsd::userland::libc {

struct Utimbuf {
    std::int64_t actime{0};
    std::int64_t modtime{0};
};

[[nodiscard]] inline bool utimbuf_valid(const Utimbuf& u) noexcept {
    return u.actime >= 0 && u.modtime >= 0;
}

} // namespace pbsd::userland::libc
