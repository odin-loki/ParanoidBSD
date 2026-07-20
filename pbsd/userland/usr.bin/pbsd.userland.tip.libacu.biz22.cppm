module;

export module pbsd.userland.tip.libacu.biz22;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/biz22.c
export namespace pbsd::userland::usr_bin::tip::libacu::biz22 {

[[nodiscard]] inline bool libacu_biz22_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::biz22
