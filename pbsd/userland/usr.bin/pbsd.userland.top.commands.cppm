module;

export module pbsd.userland.top.commands;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/top/commands.c
export namespace pbsd::userland::usr_bin::top::commands {

[[nodiscard]] inline bool commands_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::top::commands
