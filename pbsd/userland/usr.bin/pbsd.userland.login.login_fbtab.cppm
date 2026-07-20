module;

export module pbsd.userland.login.login_fbtab;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/login/login_fbtab.c
export namespace pbsd::userland::usr_bin::login::login_fbtab {

[[nodiscard]] inline bool login_fbtab_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::login::login_fbtab
