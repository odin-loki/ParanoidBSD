module;

export module pbsd.userland.tip.tip.acu;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/acu.c
export namespace pbsd::userland::usr_bin::tip::tip::acu {

[[nodiscard]] inline bool tip_acu_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::acu
