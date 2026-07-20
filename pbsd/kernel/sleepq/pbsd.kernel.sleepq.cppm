export module pbsd.kernel.sleepq;

export import pbsd.core;

/// Wave 4 — sleepqueue stubs from sys/sleepqueue.h.
export namespace pbsd::kernel::sleepq {

inline constexpr int kTypeNone   = 0;
inline constexpr int kTypeSleep  = 1;
inline constexpr int kTypeCondvar = 2;
inline constexpr int kSleepOk    = 0;
inline constexpr int kSleepIntr  = 1;
inline constexpr int kSleepTimeout = 2;

struct Channel {
    const void* wchan{};
    int type{kTypeNone};
    unsigned waiters{};
};

[[nodiscard]] constexpr Status sleep(Channel& ch, const void* wchan, int type) noexcept {
    if (wchan == nullptr) {
        return Status::Invalid;
    }
    ch.wchan = wchan;
    ch.type = type;
    ++ch.waiters;
    return Status::Ok;
}

[[nodiscard]] constexpr Status wakeup(Channel& ch) noexcept {
    if (ch.waiters == 0) {
        return Status::NotFound;
    }
    ch.waiters = 0;
    ch.wchan = nullptr;
    ch.type = kTypeNone;
    return Status::Ok;
}

} // namespace pbsd::kernel::sleepq
