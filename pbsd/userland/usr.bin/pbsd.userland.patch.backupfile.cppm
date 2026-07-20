module;

export module pbsd.userland.patch.backupfile;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/patch/backupfile.c
export namespace pbsd::userland::usr_bin::patch::backupfile {

[[nodiscard]] inline bool backupfile_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::patch::backupfile
