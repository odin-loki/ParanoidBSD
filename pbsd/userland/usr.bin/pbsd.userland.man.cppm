module;

export module pbsd.userland.man;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/man/man.c
export namespace pbsd::userland::usr_bin::man {

[[nodiscard]] inline bool man_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::man
