module;

export module pbsd.userland.mail.strings;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/strings.c
export namespace pbsd::userland::usr_bin::mail::strings {

[[nodiscard]] inline bool strings_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::strings
