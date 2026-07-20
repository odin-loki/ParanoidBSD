module;

export module pbsd.userland.mail.quit;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/quit.c
export namespace pbsd::userland::usr_bin::mail::quit {

[[nodiscard]] inline bool quit_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::quit
