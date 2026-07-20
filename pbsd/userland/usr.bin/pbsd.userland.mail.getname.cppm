module;

export module pbsd.userland.mail.getname;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/getname.c
export namespace pbsd::userland::usr_bin::mail::getname {

[[nodiscard]] inline bool getname_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::getname
