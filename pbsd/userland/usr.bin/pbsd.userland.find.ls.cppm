module;

export module pbsd.userland.find.ls;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/find/ls.c
export namespace pbsd::userland::usr_bin::find::ls {

[[nodiscard]] inline bool ls_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::find::ls
