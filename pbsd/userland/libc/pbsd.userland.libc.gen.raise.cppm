module;

export module pbsd.userland.libc.gen.raise;

/// raise(3) signal dispatch concept from hbsd/src/lib/libc/gen/raise.c
export namespace pbsd::userland::libc {

using SignalHandler = void (*)(int) noexcept;

inline SignalHandler g_signal_handler = nullptr;

inline void set_signal_handler(SignalHandler h) noexcept { g_signal_handler = h; }

[[nodiscard]] inline int raise_signal(int sig) noexcept {
    if (g_signal_handler != nullptr) {
        g_signal_handler(sig);
        return 0;
    }
    return -1;
}

} // namespace pbsd::userland::libc
