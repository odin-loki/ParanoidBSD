module;

export module pbsd.userland.tip.tip.tip;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/tip.c
export namespace pbsd::userland::usr_bin::tip::tip::tip {

[[nodiscard]] inline bool tip_tip_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::tip
