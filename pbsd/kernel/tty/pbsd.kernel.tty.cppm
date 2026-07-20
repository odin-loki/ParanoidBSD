export module pbsd.kernel.tty;

export import pbsd.core;

/// Wave 4 — tty line discipline / termios flags from sys/tty.h, sys/termios.h.
export namespace pbsd::kernel::tty {

inline constexpr unsigned kNccs = 20;

inline constexpr unsigned kIflagIgnbrk = 0x0001;
inline constexpr unsigned kIflagBrkint = 0x0002;
inline constexpr unsigned kIflagInlcr  = 0x0040;
inline constexpr unsigned kIflagIcrnl  = 0x0100;

inline constexpr unsigned kOflagOpost = 0x0001;
inline constexpr unsigned kOflagOnlcr = 0x0002;

inline constexpr unsigned kLflagEcho  = 0x0008;
inline constexpr unsigned kLflagIcanon = 0x0100;
inline constexpr unsigned kLflagIsig  = 0x0080;

struct TermiosStub {
    unsigned iflag{};
    unsigned oflag{};
    unsigned cflag{};
    unsigned lflag{};
    unsigned char cc[kNccs]{};
};

[[nodiscard]] constexpr bool canonical(const TermiosStub& t) noexcept {
    return (t.lflag & kLflagIcanon) != 0;
}

[[nodiscard]] constexpr Status validate_termios(const TermiosStub& t) noexcept {
    (void)t;
    return Status::Ok;
}

} // namespace pbsd::kernel::tty
