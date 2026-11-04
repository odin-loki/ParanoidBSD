module;

export module pbsd.userland.tip.tip.cmdtab;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/cmdtab.c
export namespace pbsd::userland::usr_bin::tip::tip::cmdtab {

[[nodiscard]] inline bool tip_cmdtab_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::cmdtab
