module;

export module pbsd.userland.mail.fio;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/fio.c
export namespace pbsd::userland::usr_bin::mail::fio {

[[nodiscard]] inline bool fio_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::fio
