module;

export module pbsd.userland.tip.libacu.courier;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/courier.c
export namespace pbsd::userland::usr_bin::tip::libacu::courier {

[[nodiscard]] inline bool libacu_courier_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::courier
