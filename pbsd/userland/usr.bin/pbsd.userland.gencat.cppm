module;

export module pbsd.userland.gencat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/gencat/gencat.c
export namespace pbsd::userland::usr_bin::gencat {

[[nodiscard]] inline Status gencat_output(const char* out) noexcept {
    if (out == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::gencat
