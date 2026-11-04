module;

export module pbsd.userland.mail.lex;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/lex.c
export namespace pbsd::userland::usr_bin::mail::lex {

[[nodiscard]] inline bool lex_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::lex
