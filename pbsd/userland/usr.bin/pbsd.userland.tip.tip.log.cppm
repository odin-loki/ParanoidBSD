module;

export module pbsd.userland.tip.tip.log;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/log.c
export namespace pbsd::userland::usr_bin::tip::tip::log {

[[nodiscard]] inline bool tip_log_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::log
