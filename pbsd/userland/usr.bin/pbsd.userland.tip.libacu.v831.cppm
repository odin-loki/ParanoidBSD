module;

export module pbsd.userland.tip.libacu.v831;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/v831.c
export namespace pbsd::userland::usr_bin::tip::libacu::v831 {

[[nodiscard]] inline bool libacu_v831_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::v831
