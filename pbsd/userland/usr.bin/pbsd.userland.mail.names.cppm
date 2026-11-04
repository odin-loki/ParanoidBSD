module;

export module pbsd.userland.mail.names;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/names.c
export namespace pbsd::userland::usr_bin::mail::names {

[[nodiscard]] inline bool names_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::names
