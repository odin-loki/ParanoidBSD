module;

export module pbsd.userland.libc.locale.table;

export import pbsd.core;

/// table scaffold from hbsd/src/lib/libc/locale/table.c
export namespace pbsd::userland::libc::locale {

struct table_state {
    int initialized = 0;
};

[[nodiscard]] inline StatusOnly table_init(table_state* st) noexcept {
    if (st == nullptr) {
        return status_err(Status::Invalid);
    }
    st->initialized = 1;
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::locale
