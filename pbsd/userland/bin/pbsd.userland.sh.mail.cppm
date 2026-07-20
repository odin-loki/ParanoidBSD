module;

export module pbsd.userland.sh.mail;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/sh/mail.c
export namespace pbsd::userland::bin::sh::mail {

[[nodiscard]] inline bool mail_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::bin::sh::mail
