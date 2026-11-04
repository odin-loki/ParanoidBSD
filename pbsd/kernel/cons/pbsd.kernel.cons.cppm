module;
#include <cstdint>

export module pbsd.kernel.cons;

export import pbsd.core;

/// Wave 4/5 — console layer from sys/cons.h, kern/tty_cons.c.
export namespace pbsd::kernel::cons {

inline constexpr int kConsPollhz = 4;
inline constexpr int kConsBufsize = 127;

inline constexpr unsigned kConsFlagsEnabled  = 0x0001;
inline constexpr unsigned kConsFlagsMute     = 0x0002;
inline constexpr unsigned kConsFlagsPanic    = 0x0004;
inline constexpr unsigned kConsFlagsRedirect = 0x0008;
inline constexpr unsigned kConsFlagsCtrl     = 0x0010;

enum class ConsoleType : int {
    Unknown = 0,
    Tty     = 1,
    Serial  = 2,
    Vt      = 3,
};

struct ConsoleState {
    ConsoleType type{ConsoleType::Unknown};
    unsigned    flags{};
    int         poll_hz{kConsPollhz};
    bool        attached{};
};

[[nodiscard]] constexpr bool is_enabled(unsigned flags) noexcept {
    return (flags & kConsFlagsEnabled) != 0;
}

[[nodiscard]] constexpr bool is_muted(unsigned flags) noexcept {
    return (flags & kConsFlagsMute) != 0;
}

[[nodiscard]] constexpr Status validate_poll_hz(int hz) noexcept {
    if (hz <= 0 || hz > 1000) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] constexpr unsigned enable(unsigned flags) noexcept {
    return flags | kConsFlagsEnabled;
}

[[nodiscard]] constexpr unsigned mute(unsigned flags) noexcept {
    return flags | kConsFlagsMute;
}

[[nodiscard]] constexpr unsigned unmute(unsigned flags) noexcept {
    return flags & ~kConsFlagsMute;
}

} // namespace pbsd::kernel::cons
