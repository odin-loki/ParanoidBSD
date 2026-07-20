module;

export module pbsd.userland.libc.locale.ldpart;

export import pbsd.core;

/// ldpart scaffold from hbsd/src/lib/libc/locale/ldpart.c
export namespace pbsd::userland::libc::locale {

struct ldpart_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly ldpart_init(ldpart_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
