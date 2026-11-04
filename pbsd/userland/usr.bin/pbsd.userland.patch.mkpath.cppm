module;

export module pbsd.userland.patch.mkpath;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/patch/mkpath.c
export namespace pbsd::userland::usr_bin::patch::mkpath {

[[nodiscard]] inline bool mkpath_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::patch::mkpath
