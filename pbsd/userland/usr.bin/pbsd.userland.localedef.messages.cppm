module;

export module pbsd.userland.localedef.messages;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/localedef/messages.c
export namespace pbsd::userland::usr_bin::localedef::messages {

[[nodiscard]] inline bool messages_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::localedef::messages
