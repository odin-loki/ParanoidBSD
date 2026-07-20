module;

export module pbsd.userland.host;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/host/host.c
export namespace pbsd::userland::usr_bin::host {

[[nodiscard]] inline Status host_validate_name(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::host
