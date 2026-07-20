module;

export module pbsd.userland.mail.vars;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/vars.c
export namespace pbsd::userland::usr_bin::mail::vars {

[[nodiscard]] inline bool vars_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::vars
