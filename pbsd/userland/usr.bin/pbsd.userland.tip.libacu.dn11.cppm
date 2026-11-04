module;

export module pbsd.userland.tip.libacu.dn11;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/dn11.c
export namespace pbsd::userland::usr_bin::tip::libacu::dn11 {

[[nodiscard]] inline bool libacu_dn11_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::dn11
