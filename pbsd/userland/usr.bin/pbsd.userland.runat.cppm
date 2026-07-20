module;

export module pbsd.userland.runat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/runat/runat.c
export namespace pbsd::userland::usr_bin::runat {

[[nodiscard]] inline Status runat_dir(const char* dir) noexcept {
    if (dir == nullptr || dir[0] == '\0') {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::usr_bin::runat
