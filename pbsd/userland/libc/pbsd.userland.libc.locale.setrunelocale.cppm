module;

export module pbsd.userland.libc.locale.setrunelocale;

export import pbsd.core;

/// setrunelocale scaffold from hbsd/src/lib/libc/locale/setrunelocale.c
export namespace pbsd::userland::libc::locale {

struct setrunelocale_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly setrunelocale_init(setrunelocale_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
