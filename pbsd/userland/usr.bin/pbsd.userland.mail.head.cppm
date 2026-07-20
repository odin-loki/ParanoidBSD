module;

export module pbsd.userland.mail.head;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/head.c
export namespace pbsd::userland::usr_bin::mail::head {

[[nodiscard]] inline bool head_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::head
