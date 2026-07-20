module;

export module pbsd.userland.libc.locale.lnumeric;

export import pbsd.core;

/// lnumeric scaffold from hbsd/src/lib/libc/locale/lnumeric.c
export namespace pbsd::userland::libc::locale {

struct lnumeric_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly lnumeric_init(lnumeric_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
