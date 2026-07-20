module;

export module pbsd.userland.mail.temp;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/temp.c
export namespace pbsd::userland::usr_bin::mail::temp {

[[nodiscard]] inline bool temp_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::temp
