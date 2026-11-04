module;

export module pbsd.userland.ts;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/ts/ts.c
export namespace pbsd::userland::usr_bin::ts {

[[nodiscard]] inline bool ts_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::ts
