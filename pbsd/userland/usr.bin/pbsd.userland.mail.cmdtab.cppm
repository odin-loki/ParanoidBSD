module;

export module pbsd.userland.mail.cmdtab;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/cmdtab.c
export namespace pbsd::userland::usr_bin::mail::cmdtab {

[[nodiscard]] inline bool cmdtab_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::cmdtab
