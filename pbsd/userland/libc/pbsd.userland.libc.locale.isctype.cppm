module;

export module pbsd.userland.libc.locale.isctype;

export import pbsd.core;

/// isctype scaffold from hbsd/src/lib/libc/locale/isctype.c
export namespace pbsd::userland::libc::locale {

struct isctype_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly isctype_init(isctype_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
