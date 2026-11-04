module;
#include <cstdint>

export module pbsd.kernel.rmlock;

export import pbsd.core;

/// Wave 4/5 — rmlock init flags from sys/rmlock.h.
export namespace pbsd::kernel::rmlock {

inline constexpr unsigned kNowitness  = 0x00000001;
inline constexpr unsigned kRecurse    = 0x00000002;
inline constexpr unsigned kSleepable  = 0x00000004;
inline constexpr unsigned kNew        = 0x00000008;
inline constexpr unsigned kDupok      = 0x00000010;

struct RmlockStub {
    unsigned opts{};
    bool     write_held{};
    unsigned readers{};
};

[[nodiscard]] constexpr bool is_sleepable(unsigned opts) noexcept {
    return (opts & kSleepable) != 0;
}

[[nodiscard]] constexpr Status validate_opts(unsigned opts) noexcept {
    if ((opts & ~(kNowitness | kRecurse | kSleepable | kNew | kDupok)) != 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::rmlock
