module;

export module pbsd.userland.tip.tip.partab;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/partab.c
export namespace pbsd::userland::usr_bin::tip::tip::partab {

[[nodiscard]] inline bool tip_partab_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::partab
