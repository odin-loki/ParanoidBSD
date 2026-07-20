module;

export module pbsd.userland.libc.locale.lmessages;

export import pbsd.core;

/// lmessages scaffold from hbsd/src/lib/libc/locale/lmessages.c
export namespace pbsd::userland::libc::locale {

struct lmessages_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly lmessages_init(lmessages_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
