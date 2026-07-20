module;

export module pbsd.userland.libc.gen.nice;

export import pbsd.core;

/// nice from hbsd/src/lib/libc/gen/nice.c
export namespace pbsd::userland::libc {

inline constexpr int kMinNice = -20;
inline constexpr int kMaxNice = 20;

[[nodiscard]] inline Status validate_nice(int incr) noexcept {
    if (incr < kMinNice || incr > kMaxNice) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline int clamp_nice(int prio) noexcept {
    if (prio < kMinNice) {
        return kMinNice;
    }
    if (prio > kMaxNice) {
        return kMaxNice;
    }
    return prio;
}

} // namespace pbsd::userland::libc
