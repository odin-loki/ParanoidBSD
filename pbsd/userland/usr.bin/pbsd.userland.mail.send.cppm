module;

export module pbsd.userland.mail.send;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/mail/send.c
export namespace pbsd::userland::usr_bin::mail::send {

[[nodiscard]] inline bool send_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::mail::send
