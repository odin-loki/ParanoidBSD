export module pbsd.kernel.poll;

export import pbsd.core;

/// Wave 4 — poll(2) event bits from sys/poll.h.
export namespace pbsd::kernel::poll {

enum class Event : short {
    None = 0,
    In = 0x0001,     // POLLIN
    Pri = 0x0002,    // POLLPRI
    Out = 0x0004,    // POLLOUT
    Err = 0x0008,    // POLLERR
    Hup = 0x0010,    // POLLHUP
    Nval = 0x0020,   // POLLNVAL
    RdNorm = 0x0040, // POLLRDNORM
    RdBand = 0x0080, // POLLRDBAND
    WrNorm = 0x0100, // POLLWRNORM
    WrBand = 0x0200, // POLLWRBAND
};

[[nodiscard]] constexpr Event operator|(Event a, Event b) noexcept {
    return static_cast<Event>(static_cast<short>(a) | static_cast<short>(b));
}

[[nodiscard]] constexpr bool has(Event set, Event bit) noexcept {
    return (static_cast<short>(set) & static_cast<short>(bit)) != 0;
}

struct PollFd {
    int fd{-1};
    Event events{Event::None};
    Event revents{Event::None};
};

[[nodiscard]] constexpr Status validate_fd(const PollFd& p) noexcept {
    if (p.fd < 0 && p.fd != -1) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::kernel::poll
