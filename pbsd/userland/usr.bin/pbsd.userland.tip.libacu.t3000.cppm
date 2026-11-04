module;

export module pbsd.userland.tip.libacu.t3000;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/t3000.c
export namespace pbsd::userland::usr_bin::tip::libacu::t3000 {

[[nodiscard]] inline bool libacu_t3000_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::t3000
