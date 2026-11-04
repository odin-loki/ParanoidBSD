module;

export module pbsd.userland.chat;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/chat/chat.c
export namespace pbsd::userland::usr_bin::chat {

[[nodiscard]] inline bool chat_echo(char flag) noexcept { return flag == 'e'; }

} // namespace pbsd::userland::usr_bin::chat
