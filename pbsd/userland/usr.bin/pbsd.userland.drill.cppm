module;

export module pbsd.userland.drill;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/drill/drill.c
export namespace pbsd::userland::usr_bin::drill {

[[nodiscard]] inline bool drill_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::drill
