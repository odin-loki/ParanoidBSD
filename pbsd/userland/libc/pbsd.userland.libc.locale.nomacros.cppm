module;

export module pbsd.userland.libc.locale.nomacros;

export import pbsd.core;

/// nomacros scaffold from hbsd/src/lib/libc/locale/nomacros.c
export namespace pbsd::userland::libc::locale {

struct nomacros_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly nomacros_init(nomacros_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
