module;

export module pbsd.userland.mail.version;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/version.c
export namespace pbsd::userland::usr_bin::mail::version {

[[nodiscard]] inline bool version_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::version
