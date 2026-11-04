module;

export module pbsd.userland.tip.libacu.v3451;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/v3451.c
export namespace pbsd::userland::usr_bin::tip::libacu::v3451 {

[[nodiscard]] inline bool libacu_v3451_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::v3451
