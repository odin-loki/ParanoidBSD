module;

export module pbsd.userland.mail.popen;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/popen.c
export namespace pbsd::userland::usr_bin::mail::popen {

[[nodiscard]] inline bool popen_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::popen
