module;

export module pbsd.userland.rup;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rup/rup.c
export namespace pbsd::userland::usr_bin::rup {

[[nodiscard]] inline Status rup_host(const char* host) noexcept {
    if (host == nullptr || host[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::rup
