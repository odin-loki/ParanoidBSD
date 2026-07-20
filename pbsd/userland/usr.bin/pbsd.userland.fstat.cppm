module;

export module pbsd.userland.fstat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/fstat/fstat.c
export namespace pbsd::userland::usr_bin::fstat {

[[nodiscard]] inline Status fstat_fd(int fd) noexcept {
    if (fd < 0) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::fstat
