module;

export module pbsd.userland.libc.locale.collate;

export import pbsd.core;

/// collate scaffold from hbsd/src/lib/libc/locale/collate.c
export namespace pbsd::userland::libc::locale {

struct collate_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly collate_init(collate_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
