module;

export module pbsd.userland.c99;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/c99/c99.c
export namespace pbsd::userland::usr_bin::c99 {

[[nodiscard]] inline bool c99_pedantic(char flag) noexcept { return flag == 'p'; }

} // namespace pbsd::userland::usr_bin::c99
