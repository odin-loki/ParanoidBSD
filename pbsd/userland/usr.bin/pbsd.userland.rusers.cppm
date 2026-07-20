module;

export module pbsd.userland.rusers;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/rusers/rusers.c
export namespace pbsd::userland::usr_bin::rusers {

[[nodiscard]] inline bool rusers_long_format(char flag) noexcept { return flag == 'l'; }

} // namespace pbsd::userland::usr_bin::rusers
