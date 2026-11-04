module;
#include <cstdint>

export module pbsd.kernel.filter;

export import pbsd.core;

/// Freestanding port of `sys/tim_filter.h` / `kern/subr_filter.c`.
export namespace pbsd::kernel::filter {

inline constexpr unsigned kNumEntries = 3;
inline constexpr int kTypeMin = 1;
inline constexpr int kTypeMax = 2;

struct Entry {
    std::uint64_t value{};
    unsigned time_up{};
};

struct EntrySmall {
    unsigned value{};
    unsigned time_up{};
};

struct Softc {
    unsigned cur_time_limit{};
    Entry entries[kNumEntries]{};
};

struct SoftcSmall {
    unsigned cur_time_limit{};
    EntrySmall entries[kNumEntries]{};
};

[[nodiscard]] inline Status validate_type(int fil_type) noexcept {
    if (fil_type != kTypeMin && fil_type != kTypeMax) {
        return Status::Invalid;
    }
    return Status::Ok;
}

inline void reset(Softc& tf, unsigned time_len) noexcept {
    tf.cur_time_limit = time_len;
}

[[nodiscard]] inline std::uint64_t value(const Softc& tf) noexcept {
    return tf.entries[0].value;
}

} // namespace pbsd::kernel::filter
