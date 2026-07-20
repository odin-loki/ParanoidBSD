module;

export module pbsd.userland.libc.locale.none;

export import pbsd.core;

/// none scaffold from hbsd/src/lib/libc/locale/none.c
export namespace pbsd::userland::libc::locale {

struct none_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly none_init(none_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
