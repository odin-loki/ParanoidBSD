module;

export module pbsd.userland.libc.locale.xlocale;

export import pbsd.core;

/// xlocale scaffold from hbsd/src/lib/libc/locale/xlocale.c
export namespace pbsd::userland::libc::locale {

struct xlocale_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly xlocale_init(xlocale_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
