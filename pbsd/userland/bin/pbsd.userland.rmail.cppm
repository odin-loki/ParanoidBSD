module;

export module pbsd.userland.rmail;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/rmail/rmail.c
export namespace pbsd::userland::bin::rmail {

[[nodiscard]] inline Status rmail_recipient(const char* addr) noexcept {
    if (addr == nullptr || addr[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::bin::rmail
