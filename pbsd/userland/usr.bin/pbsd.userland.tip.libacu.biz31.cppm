module;

export module pbsd.userland.tip.libacu.biz31;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/biz31.c
export namespace pbsd::userland::usr_bin::tip::libacu::biz31 {

[[nodiscard]] inline bool libacu_biz31_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::biz31
