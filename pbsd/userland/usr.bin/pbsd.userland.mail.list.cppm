module;

export module pbsd.userland.mail.list;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/list.c
export namespace pbsd::userland::usr_bin::mail::list {

[[nodiscard]] inline bool list_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::list
