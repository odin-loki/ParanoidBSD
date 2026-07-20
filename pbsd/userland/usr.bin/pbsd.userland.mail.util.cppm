module;

export module pbsd.userland.mail.util;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/util.c
export namespace pbsd::userland::usr_bin::mail::util {

[[nodiscard]] inline bool util_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::util
