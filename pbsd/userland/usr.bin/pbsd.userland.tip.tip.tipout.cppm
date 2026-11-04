module;

export module pbsd.userland.tip.tip.tipout;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/tipout.c
export namespace pbsd::userland::usr_bin::tip::tip::tipout {

[[nodiscard]] inline bool tip_tipout_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::tipout
