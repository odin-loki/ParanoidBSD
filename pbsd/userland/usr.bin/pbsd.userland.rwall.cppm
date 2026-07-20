module;

export module pbsd.userland.rwall;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rwall/rwall.c
export namespace pbsd::userland::usr_bin::rwall {

[[nodiscard]] inline Status rwall_message(const char* msg) noexcept {
    if (msg == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::rwall
