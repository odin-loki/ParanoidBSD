module;

export module pbsd.userland.libc.locale.lmonetary;

export import pbsd.core;

/// lmonetary scaffold from hbsd/src/lib/libc/locale/lmonetary.c
export namespace pbsd::userland::libc::locale {

struct lmonetary_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly lmonetary_init(lmonetary_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
