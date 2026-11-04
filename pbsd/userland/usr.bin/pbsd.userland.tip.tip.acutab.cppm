module;

export module pbsd.userland.tip.tip.acutab;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/acutab.c
export namespace pbsd::userland::usr_bin::tip::tip::acutab {

[[nodiscard]] inline bool tip_acutab_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::acutab
