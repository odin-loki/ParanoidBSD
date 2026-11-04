module;

export module pbsd.userland.mail;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/cmd1.c
export namespace pbsd::userland::usr_bin::mail {

[[nodiscard]] inline bool mail_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::mail
