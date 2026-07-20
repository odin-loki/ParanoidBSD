module;

export module pbsd.userland.logins;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/logins/logins.c
export namespace pbsd::userland::usr_bin::logins {

[[nodiscard]] inline bool logins_pretty(char flag) noexcept { return flag == 'p'; }

} // namespace pbsd::userland::usr_bin::logins
