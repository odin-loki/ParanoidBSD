module;

export module pbsd.userland.libc.locale.rune;

export import pbsd.core;

/// rune scaffold from hbsd/src/lib/libc/locale/rune.c
export namespace pbsd::userland::libc::locale {

struct rune_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly rune_init(rune_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
