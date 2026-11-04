module;

export module pbsd.userland.libc.sys.open;

export import pbsd.core;

/// open from hbsd/src/lib/libc/sys/open.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status open_validate_path(const char* path) noexcept {
    if (path == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
