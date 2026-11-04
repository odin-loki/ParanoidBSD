module;

export module pbsd.userland.mail.v7.local;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/v7.local.c
export namespace pbsd::userland::usr_bin::mail::v7::local {

[[nodiscard]] inline bool v7_local_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::v7::local
