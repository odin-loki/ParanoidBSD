module;

export module pbsd.userland.limits;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/limits/limits.c
export namespace pbsd::userland::usr_bin::limits {

[[nodiscard]] inline bool limits_show_all(char flag) noexcept { return flag == 'a'; }

} // namespace pbsd::userland::usr_bin::limits
