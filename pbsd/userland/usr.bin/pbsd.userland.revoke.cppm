module;

export module pbsd.userland.revoke;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/revoke/revoke.c
export namespace pbsd::userland::usr_bin::revoke {

[[nodiscard]] inline Status revoke_path(const char* path) noexcept {
    if (path == nullptr || path[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::revoke
