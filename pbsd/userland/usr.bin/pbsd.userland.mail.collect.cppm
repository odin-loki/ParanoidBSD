module;

export module pbsd.userland.mail.collect;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/collect.c
export namespace pbsd::userland::usr_bin::mail::collect {

[[nodiscard]] inline bool collect_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::collect
