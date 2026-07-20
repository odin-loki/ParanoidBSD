module;

export module pbsd.userland.alias;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/alias/alias.c
export namespace pbsd::userland::usr_bin::alias {

[[nodiscard]] inline bool alias_flag(char c) noexcept { return c == 'v'; }

} // namespace pbsd::userland::usr_bin::alias
