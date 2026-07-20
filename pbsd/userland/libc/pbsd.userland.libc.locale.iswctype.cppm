module;

export module pbsd.userland.libc.locale.iswctype;

export import pbsd.core;

/// iswctype scaffold from hbsd/src/lib/libc/locale/iswctype.c
export namespace pbsd::userland::libc::locale {

struct iswctype_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly iswctype_init(iswctype_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
