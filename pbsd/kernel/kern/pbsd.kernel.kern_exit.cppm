module;

export module pbsd.kernel.kern_exit;

export import pbsd.core;

/// Freestanding port of `kern/kern_exit.c` — exit/wait status encoding.
export namespace pbsd::kernel::kern_exit {

[[nodiscard]] inline int encode_exit(int code) noexcept {
    return (code & 0xff) << 8;
}

[[nodiscard]] inline int encode_signal(int sig) noexcept {
    return sig;
}

[[nodiscard]] inline bool stopped(int status) noexcept {
    return (status & 0xff) == 0x7f;
}

[[nodiscard]] inline bool exited(int status) noexcept {
    return (status & 0xff) == 0;
}

[[nodiscard]] inline int exit_code(int status) noexcept {
    return (status >> 8) & 0xff;
}

} // namespace pbsd::kernel::kern_exit
