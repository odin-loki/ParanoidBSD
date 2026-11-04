module;

export module pbsd.userland.mail.tty;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/tty.c
export namespace pbsd::userland::usr_bin::mail::tty {

[[nodiscard]] inline bool tty_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::tty
