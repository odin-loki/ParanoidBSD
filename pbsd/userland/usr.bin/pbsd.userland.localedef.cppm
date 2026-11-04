module;

export module pbsd.userland.localedef;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/localedef.c
export namespace pbsd::userland::usr_bin::localedef {

[[nodiscard]] inline bool localedef_force(char flag) noexcept { return flag == 'f'; }

} // namespace pbsd::userland::usr_bin::localedef
