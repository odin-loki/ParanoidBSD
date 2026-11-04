module;

export module pbsd.userland.mail.cmd2;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/cmd2.c
export namespace pbsd::userland::usr_bin::mail::cmd2 {

[[nodiscard]] inline bool cmd2_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::cmd2
