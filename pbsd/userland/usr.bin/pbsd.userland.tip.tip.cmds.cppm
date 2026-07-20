module;

export module pbsd.userland.tip.tip.cmds;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/cmds.c
export namespace pbsd::userland::usr_bin::tip::tip::cmds {

[[nodiscard]] inline bool tip_cmds_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::cmds
