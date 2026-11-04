module;

export module pbsd.userland.mail.cmd3;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/cmd3.c
export namespace pbsd::userland::usr_bin::mail::cmd3 {

[[nodiscard]] inline bool cmd3_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::cmd3
