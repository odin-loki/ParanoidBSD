module;

export module pbsd.userland.tip.tip.value;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/tip/tip/value.c
export namespace pbsd::userland::usr_bin::tip::tip::value {

[[nodiscard]] inline bool tip_value_flag(char c) noexcept { return c == 'v'; }
} // namespace pbsd::userland::usr_bin::tip::tip::value
