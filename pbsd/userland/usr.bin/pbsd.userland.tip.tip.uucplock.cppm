module;

export module pbsd.userland.tip.tip.uucplock;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/uucplock.c
export namespace pbsd::userland::usr_bin::tip::tip::uucplock {

[[nodiscard]] inline bool tip_uucplock_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::uucplock
