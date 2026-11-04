module;

export module pbsd.userland.tip.libacu.ventel;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/ventel.c
export namespace pbsd::userland::usr_bin::tip::libacu::ventel {

[[nodiscard]] inline bool libacu_ventel_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::ventel
