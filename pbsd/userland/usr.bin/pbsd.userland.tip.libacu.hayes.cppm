module;

export module pbsd.userland.tip.libacu.hayes;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/hayes.c
export namespace pbsd::userland::usr_bin::tip::libacu::hayes {

[[nodiscard]] inline bool libacu_hayes_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::hayes
