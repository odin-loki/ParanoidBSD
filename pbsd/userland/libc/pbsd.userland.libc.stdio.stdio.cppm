module;

export module pbsd.userland.libc.stdio.stdio;

export import pbsd.core;

/// stdio init scaffold from hbsd/src/lib/libc/stdio/stdio.c
export namespace pbsd::userland::libc::stdio {

struct IobState {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly stdio_init(IobState* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_ok();
}

} // namespace pbsd::userland::libc::stdio
