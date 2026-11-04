module;

export module pbsd.userland.tip.libacu.df;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/libacu/df.c
export namespace pbsd::userland::usr_bin::tip::libacu::df {

[[nodiscard]] inline bool libacu_df_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::libacu::df
