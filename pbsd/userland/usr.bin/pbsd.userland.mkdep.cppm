module;

export module pbsd.userland.mkdep;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mkdep/mkdep.c
export namespace pbsd::userland::usr_bin::mkdep {

[[nodiscard]] inline bool mkdep_append_suffix(const char* path) noexcept {
    return path != nullptr && path[0] != '\0';
}

} // namespace pbsd::userland::usr_bin::mkdep
